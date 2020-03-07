#![allow(clippy::needless_return)]

use crate::basis::Basis;
use crate::generate;
use crate::limits;
use crate::point::Point;
use std::time::Instant;

fn g_batch(c: f64, e: f64, gaussians: &mut [f64], p2s: &[f64]) -> () {
    for ipoint in 0..limits::BATCH_LENGTH {
        gaussians[ipoint] += c * (-e * p2s[ipoint]).exp();
    }
}

fn compute_gaussians(
    geo_derv_orders: (usize, usize, usize),
    num_batches: usize,
    l: usize,
    pxs: &Vec<f64>,
    pys: &Vec<f64>,
    pzs: &Vec<f64>,
    p2s: Vec<f64>,
    basis: &Basis,
    offset: usize,
    num_primitives: usize,
) -> Vec<f64> {
    let num_points = p2s.len();
    let mut gaussians = vec![0.0; num_points];

    for ip in offset..(offset + num_primitives) {
        let e = basis.primitive_exponents[ip];
        let c = basis.contraction_coefficients[ip];
        for ibatch in 0..num_batches {
            let start = ibatch * limits::BATCH_LENGTH;
            let end = start + limits::BATCH_LENGTH;
            g_batch(c, e, &mut gaussians[start..end], &p2s[start..end]);
        }
    }

    let mut aos_c = Vec::new();

    for (i, j, k) in generate::get_ijk_list(l).iter() {
        for ipoint in 0..pxs.len() {
            aos_c.push(
                gaussians[ipoint]
                    * pxs[ipoint].powi(*i as i32)
                    * pys[ipoint].powi(*j as i32)
                    * pzs[ipoint].powi(*k as i32),
            );
        }
    }

    return aos_c;
}

fn transform_to_spherical(
    num_points: usize,
    aos_c: &[f64],
    spherical_deg: usize,
    c_to_s_matrix: &[Vec<f64>],
) -> Vec<f64> {
    let mut aos_s = vec![0.0; spherical_deg * num_points];

    for (i, row) in c_to_s_matrix.iter().enumerate() {
        let ioff = i * num_points;
        for (j, element) in row.iter().enumerate() {
            if element.abs() > std::f64::EPSILON {
                let joff = j * num_points;
                for ipoint in 0..num_points {
                    aos_s[joff + ipoint] += element * aos_c[ioff + ipoint];
                }
            }
        }
    }

    return aos_s;
}

fn coordinates(
    shell_centers_coordinates: (f64, f64, f64),
    points_bohr: &[Point],
) -> (Vec<f64>, Vec<f64>, Vec<f64>, Vec<f64>) {
    let (x, y, z) = shell_centers_coordinates;

    let mut pxs = Vec::new();
    let mut pys = Vec::new();
    let mut pzs = Vec::new();
    let mut p2s = Vec::new();

    for point in points_bohr.iter() {
        let px = point.x - x;
        let py = point.y - y;
        let pz = point.z - z;
        let p2 = px * px + py * py + pz * pz;

        pxs.push(px);
        pys.push(py);
        pzs.push(pz);
        p2s.push(p2);
    }

    return (pxs, pys, pzs, p2s);
}

pub fn aos_noddy(
    geo_derv_orders: (usize, usize, usize),
    points_bohr: &Vec<Point>,
    basis: &Basis,
    c_to_s_matrices: &[Vec<Vec<f64>>],
) -> Vec<f64> {
    let num_points = points_bohr.len();

    assert!(
        num_points % limits::BATCH_LENGTH == 0,
        "num_points must be multiple of BATCH_LENGTH"
    );
    let num_batches = num_points / limits::BATCH_LENGTH;

    let mut offset = 0;
    let mut aos = Vec::new();

    let mut time_ms_gaussian: u128 = 0;
    let mut time_ms_transform: u128 = 0;

    for ishell in 0..basis.num_shells {
        let (pxs, pys, pzs, p2s) =
            coordinates(basis.shell_centers_coordinates[ishell], &points_bohr);

        let num_primitives = basis.shell_num_primitives[ishell];
        let l = basis.shell_l_quantum_numbers[ishell];

        let timer = Instant::now();
        // TODO create shortcut for s functions when multiplying
        let mut aos_c = compute_gaussians(
            geo_derv_orders,
            num_batches,
            l,
            &pxs,
            &pys,
            &pzs,
            p2s,
            &basis,
            offset,
            num_primitives,
        );
        time_ms_gaussian += timer.elapsed().as_millis();

        let timer = Instant::now();
        if l < 2 {
            aos.append(&mut aos_c);
        } else {
            let mut aos_s = transform_to_spherical(
                num_points,
                &aos_c,
                basis.spherical_deg[ishell],
                &c_to_s_matrices[l],
            );
            aos.append(&mut aos_s);
        }
        time_ms_transform += timer.elapsed().as_millis();

        offset += num_primitives;
    }

    println!("time spent in exp: {} ms", time_ms_gaussian);
    println!("time spent in transform: {} ms", time_ms_transform);

    return aos;
}
