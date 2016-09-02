#ifndef GRID_H_INCLUDED
#define GRID_H_INCLUDED

class Main
{
    public:

        Main();
        ~Main();

//      int generate(const double radial_precision,
//                   const int    min_num_angular_points,
//                   const int    max_num_angular_points,
//                   const int    num_centers,
//                   const double center_coordinates[],
//                   const int    center_elements[],
//                   const int    num_outer_centers,
//                   const double outer_center_coordinates[],
//                   const int    outer_center_elements[],
//                   const int    num_shells,
//                   const int    shell_centers[],
//                   const int    shell_l_quantum_numbers[],
//                   const int    shell_num_primitives[],
//                   const double primitive_exponents[]);

//      int get_num_points() const;
//      double *get_grid() const;

    private:

        Main(const Main &rhs);            // not implemented
        Main &operator=(const Main &rhs); // not implemented

//      void nullify();

//      int get_closest_num_angular(int n) const;
//      int get_angular_order(int n) const;

//      int num_points;
//      double *xyzw;
};

#endif
