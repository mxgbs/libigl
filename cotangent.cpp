#include "cotangent.h"

#include "verbose.h"
#include <Eigen/Dense>

template <class MatV, class MatF, class MatC>
IGL_INLINE void igl::cotangent(const MatV & V, const MatF & F, MatC & C)
{
  using namespace igl;
  using namespace std;
  using namespace Eigen;
  // simplex size (3: triangles, 4: tetrahedra)
  int simplex_size = F.cols();
  // Number of elements
  int m = F.rows();

  if(simplex_size == 3)
  {
    // Triangles
    // edge lengths numbered same as opposite vertices
    Matrix<typename MatC::Scalar,Dynamic,3> l(m,3);
    // loop over faces
    for(int i = 0;i<m;i++)
    {
      l(i,0) = sqrt((V.row(F(i,1))-V.row(F(i,2))).array().pow(2).sum());
      l(i,1) = sqrt((V.row(F(i,2))-V.row(F(i,0))).array().pow(2).sum());
      l(i,2) = sqrt((V.row(F(i,0))-V.row(F(i,1))).array().pow(2).sum());
    }
    // semiperimeters
    Matrix<typename MatC::Scalar,Dynamic,1> s = l.rowwise().sum()*0.5;
    assert(s.rows() == m);
    // Heron's forumal for area
    Matrix<typename MatC::Scalar,Dynamic,1> dblA(m);
    for(int i = 0;i<m;i++)
    {
      dblA(i) = 2.0*sqrt(s(i)*(s(i)-l(i,0))*(s(i)-l(i,1))*(s(i)-l(i,2)));
    }
    // cotangents and diagonal entries for element matrices
    // correctly divided by 4 (alec 2010)
    C.resize(m,3);
    for(int i = 0;i<m;i++)
    {
      C(i,0) = (l(i,1)*l(i,1) + l(i,2)*l(i,2) - l(i,0)*l(i,0))/dblA(i)/4.0;
      C(i,1) = (l(i,2)*l(i,2) + l(i,0)*l(i,0) - l(i,1)*l(i,1))/dblA(i)/4.0;
      C(i,2) = (l(i,0)*l(i,0) + l(i,1)*l(i,1) - l(i,2)*l(i,2))/dblA(i)/4.0;
    }
  }else if(simplex_size == 4)
  {
    // Tetrahedra
    typedef Matrix<typename MatC::Scalar,3,1> Vec3;
    typedef Matrix<typename MatC::Scalar,3,3> Mat3;
    typedef Matrix<typename MatC::Scalar,3,4> Mat3x4;
    typedef Matrix<typename MatC::Scalar,4,4> Mat4x4;

    // preassemble right hand side
    // COLUMN-MAJOR ORDER FOR LAPACK
    Mat3x4 rhs;
    rhs <<
      1,0,0,-1,
      0,1,0,-1,
      0,0,1,-1;

    bool diag_all_pos = true;
    C.resize(m,6);

    // loop over tetrahedra
    for(int j = 0;j<F.rows();j++)
    {
      // points a,b,c,d make up the tetrahedra
      size_t a = F(j,0);
      size_t b = F(j,1);
      size_t c = F(j,2);
      size_t d = F(j,3);
      //const std::vector<double> & pa = vertices[a];
      //const std::vector<double> & pb = vertices[b];
      //const std::vector<double> & pc = vertices[c];
      //const std::vector<double> & pd = vertices[d];
      Vec3 pa = V.row(a);
      Vec3 pb = V.row(b);
      Vec3 pc = V.row(c);
      Vec3 pd = V.row(d);

      // Following definition that appears in the appendix of: ``Interactive
      // Topology-aware Surface Reconstruction,'' by Sharf, A. et al
      // http://www.cs.bgu.ac.il/~asharf/Projects/InSuRe/Insure_siggraph_final.pdf

      // compute transpose of jacobian Jj
      Mat3 JTj;
      JTj.row(0) = pa-pd;
      JTj.row(1) = pb-pd;
      JTj.row(2) = pc-pd;

      // compute abs(determinant of JTj)/6 (volume of tet)
      // determinant of transpose of A equals determinant of A
      double volume = fabs(JTj.determinant())/6.0;
      //printf("volume[%d] = %g\n",j+1,volume);

      // solve Jj' * Ej = [-I -1], for Ej
      // in other words solve JTj * Ej = [-I -1], for Ej
      Mat3x4 Ej = JTj.inverse() * rhs;
      // compute Ej'*Ej
      Mat4x4 EjTEj = Ej.transpose() * Ej;

      // Kj =  det(JTj)/6 * Ej'Ej 
      Mat4x4 Kj = EjTEj*volume;
      diag_all_pos &= Kj(0,0)>0 & Kj(1,1)>0 & Kj(2,2)>0 & Kj(3,3)>0;
      C(j,0) = Kj(1,2);
      C(j,1) = Kj(2,0);
      C(j,2) = Kj(0,1);
      C(j,3) = Kj(3,0);
      C(j,4) = Kj(3,1);
      C(j,5) = Kj(3,2);
    }
    if(diag_all_pos)
    {
      verbose("cotangent.h: Flipping sign of cotangent, so that cots are positive\n");
      C *= -1.0;
    }
  }else
  {
    fprintf(stderr,
      "cotangent.h: Error: Simplex size (%d) not supported\n", simplex_size);
    assert(false);
  }
}

#ifndef IGL_HEADER_ONLY
// Explicit template specialization
#endif