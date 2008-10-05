#ifndef __GEMATRIX_H
#define __GEMATRIX_H

namespace GE
{
  
  /*
  -------------------------------------------------
  Generic square matrix
  -------------------------------------------------
  */
  
  #ifdef minor
  # undef minor
  #endif
  
  template <int s>
  class SquareMatrix
  {
  public:
    Float m[s][s];
    
    inline static int Size () { return s; }
    
    inline Float get (int x, int y) const
    {
      return m[x][y];
    }
    
    void setIdentity ()
    {
      for (int x=0; x<s; ++x)
        for (int y=0; y<s; ++y)
          m[x][y] = (x == y) ? 1.0f : 0.0f;
    }
    
    SquareMatrix<s>& operator*= (Float k)
    {
      //Each element multiplied by k
      for (int x=0; x<s; ++x)
        for (int y=0; y<s; ++y)
          m[x][y] *= k;
      
      return *this;
    }
    
    SquareMatrix<s>& operator/= (Float k)
    {
      //Each element divided by k
      for (int x=0; x<s; ++x)
        for (int y=0; y<s; ++y)
          m[x][y] /= k;
      
      return *this;
    }
    
    SquareMatrix<s> operator* (Float k) const
    {
      SquareMatrix<s> out = *this;
      out *= k;
      return out;
    }
    
    SquareMatrix<s> operator/ (Float k) const
    {
      SquareMatrix<s> out = *this;
      out /= k;
      return out;
    }
    
    SquareMatrix<s> transpose () const
    {
      //Rows and columns swaped
      SquareMatrix<s> transmat;
      for (int x=0; x<s; ++x)
        for (int y=0; y<s; ++y)
          transmat.m [x][y] = m [y][x];
      
      return transmat;
    }
    
    SquareMatrix<s-1> submatrix (int xx, int yy) const
    {
      //A smaller matrix with x-column and y-row removed
      int x, y, subx, suby;
      SquareMatrix<s-1> submat;
      
      for (x=0, subx=0; x<s; ++x) {
        if (x == xx) continue;
        
        for (y=0, suby=0; y<s; ++y) {
          if (y == yy) continue;
          
          submat.m [subx][suby] = m [x][y];
          
          ++suby;}
        ++subx;}
      
      return submat;
    }
    
    Float minor (int x, int y) const
    {
      //Determinant of the (x,y) submatrix
      return submatrix (x,y) .determinant();
    }
    
    Float cofactor (int x, int y) const
    {
      //-1 to the power of (x+y) * minor (x,y)
      return (((x+y) % 2) ? -1 : 1) * minor (x,y);
    }
    
    Float determinant () const
    {
      if (s == 2)
      {
        //Trivial for a 2x2 matrix
        return (m[0][0] * m[1][1] -
                m[0][1] * m[1][0]);
      }
      else
      {
        //Sum of element * cofactor for the top row
        Float det = 0.0f;
        for (int x=0; x<s; ++x)
          det += m[x][0] * cofactor (x,0);
        
        return det;
      }
    }
    
    SquareMatrix<s> cofactors() const
    {
      //A matrix of cofactor elements
      SquareMatrix<s> cofmat;
      for (int x=0; x<s; ++x)
        for (int y=0; y<s; ++y)
          cofmat.m [x][y] = cofactor (x,y);
      
      return cofmat;
    }
    
    SquareMatrix<s> adjoint () const
    {
      return cofactors().transpose();
    }
    
    SquareMatrix<s> inverse () const
    {
      return adjoint() / determinant();
    }
  };
  
  template <>
  class GE_API_ENTRY SquareMatrix<1>
  {
  public:
    Float m[1][1];
    
    Float determinant () const
    {
      return m[0][0];
    }
  };
  
  /*
  ---------------------------------------------
  4x4 Transformation Matrix
  ---------------------------------------------
  */
  
  class GE_API_ENTRY Matrix4x4 : public SquareMatrix <4>
  {
  public:
    Matrix4x4 ();
    Matrix4x4 (const SquareMatrix<4> &mat);
    
    void setIdentity ();
    void setRotationX (Float radang);
    void setRotationY (Float radang);
    void setRotationZ (Float radang);
    
    void setTranslation (Float x, Float y, Float z);
    void setTranslation (const Vector3 &v);
    
    void setScale (Float x, Float y, Float z);
    void setScale (const Vector3 &v);
    void setScale (Float k);
    
    void set (Float m00, Float m10, Float m20, Float m30,
              Float m01, Float m11, Float m21, Float m31,
              Float m02, Float m12, Float m22, Float m32,
              Float m03, Float m13, Float m23, Float m33);
    
    Vector4 getColumn (int col);
    void setColumn (int col, const Vector4 &v);
    void set (const Vector4 &col1,
              const Vector4 &col2,
              const Vector4 &col3,
              const Vector4 &col4);
    
    void fromAxisAngle (Float x, Float y, Float z, Float radang);
    void fromAxisAngle (const Vector3 &axis, Float radang);
    
    void fromQuat (Float x, Float y, Float z, Float w);
    void fromQuat (const Quat &q);
    Quat toQuat ();
    
    Matrix4x4& operator*= (const Matrix4x4 &r);
    Matrix4x4 operator* (const Matrix4x4 &r) const;
    Vector3 operator* (const Vector3 &v) const;
    
    //Float determinant () const;
    //Matrix4x4 inverse () const;
    Matrix4x4 affineInverse () const;
    void affineNormalize ();
  };
  
  
  /*
  ----------------------------------------------
  Inline functions
  ----------------------------------------------
  */
  
  inline void Matrix4x4::set (Float m00, Float m10, Float m20, Float m30,
                              Float m01, Float m11, Float m21, Float m31,
                              Float m02, Float m12, Float m22, Float m32,
                              Float m03, Float m13, Float m23, Float m33)
  {
    m[0][0]=m00; m[1][0]=m10; m[2][0]=m20; m[3][0]=m30;
    m[0][1]=m01; m[1][1]=m11; m[2][1]=m21; m[3][1]=m31;
    m[0][2]=m02; m[1][2]=m12; m[2][2]=m22; m[3][2]=m32;
    m[0][3]=m03; m[1][3]=m13; m[2][3]=m23; m[3][3]=m33;
  }
  
}//namespace GE
#endif//__GEMATRIX_H
