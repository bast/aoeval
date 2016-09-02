#include "Basis.h"

#include <stdio.h>
#include <math.h>

#include <algorithm>
#include <iostream>
#include <assert.h>

#include "AOBatch.h"
#include "parameters.h"
#include "autogenerated.h"
#include "MemAllocator.h"
#include "offsets.h"


AOBatch::AOBatch()
{
    nullify();

    assert(AO_BLOCK_LENGTH%AO_CHUNK_LENGTH == 0);
}


AOBatch::~AOBatch()
{
    MemAllocator::deallocate(ao);

    MemAllocator::deallocate(ao_compressed);
    MemAllocator::deallocate(ao_compressed_index);

    MemAllocator::deallocate(k_ao_compressed);
    MemAllocator::deallocate(k_ao_compressed_index);

    MemAllocator::deallocate(l_ao_compressed);
    MemAllocator::deallocate(l_ao_compressed_index);

    nullify();
}


void AOBatch::nullify()
{
    ao_length = -1;
    ao = NULL;

    ao_compressed         = NULL;
    ao_compressed_num     = -1;
    ao_compressed_index   = NULL;
    k_ao_compressed       = NULL;
    k_ao_compressed_num   = -1;
    k_ao_compressed_index = NULL;
    l_ao_compressed       = NULL;
    l_ao_compressed_num   = -1;
    l_ao_compressed_index = NULL;
}


void AOBatch::get_ao(const Basis &basis,
                     const bool   use_gradient,
                     const int    max_ao_geo_order,
                     const int    block_length,
                     const double p[])
{
    assert(max_ao_geo_order <= MAX_GEO_DIFF_ORDER);

//  debug
//  if (max_ao_geo_order == 1)
//  {
//      printf("use_gradient = %i\n", use_gradient);
//      printf("max_ao_geo_order = %i\n", max_ao_geo_order);
//      printf("block_length = %i\n", block_length);
//      for (int ib = 0; ib < block_length; ib++)
//      {
//          printf("grid = %e %e %e %e\n", p[ib*4 + 0],
//                                         p[ib*4 + 1],
//                                         p[ib*4 + 2],
//                                         p[ib*4 + 3]);
//      }
//  }

    int l = AO_BLOCK_LENGTH*basis.num_ao_slices*basis.num_ao_cartesian;
    if (l != ao_length) // FIXME
    {
        ao_length = l;

        size_t block_size = l;

        MemAllocator::deallocate(ao);
        ao = (double*) MemAllocator::allocate(block_size*sizeof(double));

        MemAllocator::deallocate(ao_compressed);
        ao_compressed = (double*) MemAllocator::allocate(block_size*sizeof(double));

        MemAllocator::deallocate(k_ao_compressed);
        k_ao_compressed = (double*) MemAllocator::allocate(block_size*sizeof(double));

        MemAllocator::deallocate(l_ao_compressed);
        l_ao_compressed = (double*) MemAllocator::allocate(block_size*sizeof(double));

        block_size = basis.num_ao;

        MemAllocator::deallocate(ao_compressed_index);
        ao_compressed_index = (int*) MemAllocator::allocate(block_size*sizeof(int));

        MemAllocator::deallocate(k_ao_compressed_index);
        k_ao_compressed_index = (int*) MemAllocator::allocate(block_size*sizeof(int));

        MemAllocator::deallocate(l_ao_compressed_index);
        l_ao_compressed_index = (int*) MemAllocator::allocate(block_size*sizeof(int));
    }

    std::fill(&ao[0], &ao[ao_length], 0.0);

    // FIXME can be optimized
    // we do this because p can be shorter than 4*AO_BLOCK_LENGTH
    // we pad it by very large numbers to let the code screen them away
    double p_block[4*AO_BLOCK_LENGTH];
    std::fill(&p_block[0], &p_block[4*AO_BLOCK_LENGTH], 1.0e50);
    std::copy(&p[0], &p[4*block_length], &p_block[0]);

    for (int ishell = 0; ishell < basis.num_shells; ishell++)
    {
        get_ao_shell(ishell,
                     basis,
                     ao,
                     max_ao_geo_order,
                     p_block);
    }

//  debug
//  if (max_ao_geo_order == 1)
//  {
//      for (int islice = 0; islice < 4; islice++)
//      {
//          for (int iao = 0; iao < basis.num_ao_cartesian; iao++)
//          {
//              for (int ib = 0; ib < AO_BLOCK_LENGTH; ib++)
//              {
//                  printf("%i %i %i %e\n", islice, iao, ib, ao[islice*AO_BLOCK_LENGTH*basis.num_ao_cartesian + iao*AO_BLOCK_LENGTH + ib]);
//              }
//          }
//      }
//      exit(1);
//  }

    compress(basis,
             use_gradient,
             ao_compressed_num,
             ao_compressed_index,
             ao_compressed,
             std::vector<int>());
}


void AOBatch::get_ao_shell(const int        ishell,
                                const Basis &basis,
                                      double     ao_local[],
                                const int        max_ao_geo_order,
                                const double     p[])
{
    double px[AO_CHUNK_LENGTH];
    double py[AO_CHUNK_LENGTH];
    double pz[AO_CHUNK_LENGTH];
    double p2[AO_CHUNK_LENGTH];
    double s[AO_CHUNK_LENGTH];
    double buffer[BUFFER_LENGTH];

    int n = 0;
    for (int jshell = 0; jshell < ishell; jshell++)
    {
        n += basis.shell_num_primitives[jshell];
    }

    switch (max_ao_geo_order)
    {
        #include "aocalls.h"
        default:
            std::cout << "ERROR: get_ao order too high\n";
            exit(1);
            break;
    }
}


void AOBatch::compress(const Basis       &basis,
                            const bool             use_gradient,
                                  int              &aoc_num,
                                  int              *(&aoc_index),
                                  double           *(&aoc),
                            const std::vector<int> &coor)
{
    std::vector<int> cent;
    for (size_t j = 0; j < coor.size(); j++)
    {
        cent.push_back((coor[j] - 1)/3);
    }

    int num_slices;
    (use_gradient) ? (num_slices = 4) : (num_slices = 1);

    int n = 0;
    for (int i = 0; i < basis.num_ao; i++)
    {
        if (is_same_center(basis.get_ao_center(i), cent))
        {
            double tmax = 0.0;
            for (int ib = 0; ib < AO_BLOCK_LENGTH; ib++)
            {
                double t = fabs(ao[i*AO_BLOCK_LENGTH + ib]);
                if (t > tmax) tmax = t;
            }
            if (tmax > 1.0e-15)
            {
                aoc_index[n] = i;
                n++;
            }
        }
    }
    aoc_num = n;

    if (aoc_num == 0) return;

    int kp[3] = {0, 0, 0};
    for (size_t j = 0; j < coor.size(); j++)
    {
        kp[(coor[j] - 1)%3]++;
    }

    int off[4];
    off[0] = basis.get_geo_off(kp[0],   kp[1],   kp[2]);
    off[1] = basis.get_geo_off(kp[0]+1, kp[1],   kp[2]);
    off[2] = basis.get_geo_off(kp[0],   kp[1]+1, kp[2]);
    off[3] = basis.get_geo_off(kp[0],   kp[1],   kp[2]+1);

    for (int i = 0; i < aoc_num; i++)
    {
        for (int islice = 0; islice < num_slices; islice++)
        {
            int iuoff = off[islice];
            int icoff = islice*basis.num_ao;

            int iu = AO_BLOCK_LENGTH*(iuoff + aoc_index[i]);
            int ic = AO_BLOCK_LENGTH*(icoff + i);

            std::copy(&ao[iu], &ao[iu + AO_BLOCK_LENGTH], &aoc[ic]);
        }
    }
}


bool AOBatch::is_same_center(const int              c,
                                  const std::vector<int> &carray) const
// returns true if carray is empty (no derivatives)
{
    for (unsigned int i = 0; i < carray.size(); i++)
    {
        if (c != carray[i]) return false;
    }
    return true;
}
