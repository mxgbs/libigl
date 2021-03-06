#ifndef IGL_FLOOD_FILL_H
#define IGL_FLOOD_FILL_H
#include "igl_inline.h"
#include <Eigen/Core>
namespace igl
{
  // Given a 3D array with sparse non-nan (number?) data fill in the NaNs via
  // flood fill. This should ensure that, e.g., if data near 0 always has a band
  // (surface) of numbered and signed data, then components of nans will be
  // correctly signed.
  //
  // Inputs:
  //   res  3-long list of dimensions of grid
  //   S  res(0)*res(1)*res(2)  list of scalar values (with (many) nans), see
  //     output
  // Outputs:
  //   S  flood fill data in place
  IGL_INLINE void flood_fill(const Eigen::RowVector3i& res, Eigen::VectorXd& S);
}
#ifndef IGL_STATIC_LIBRARY
#  include "flood_fill.cpp"
#endif
#endif
