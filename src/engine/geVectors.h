#ifndef __VECTORS_H
#define __VECTORS_H

#include "util/geUtil.h"

namespace GE
{ 
  /*
  ------------------------------------
  Forward declarations
  ------------------------------------*/

  class Vector3;
  class Vector4;
  class Matrix4x4;

  /*
  =============================================
  2, 3, and 4-dimensional vectors
  =============================================*/
  
  class GE_API_ENTRY Vector2
  {
  public:
    Float x, y;
    
    Vector3 xy (Float z) const;
    Vector4 xy (Float z, Float w) const;
    
    Vector2 ();
    Vector2 (Float x0, Float y0);
    Vector2& set (Float x0, Float y0);
    Vector2& operator= (const Vector2 &v);
    bool operator== (const Vector2 &v) const;
    bool operator!= (const Vector2 &v) const;
    
    Vector2& operator+= (const Vector2 &v);
    Vector2& operator-= (const Vector2 &v);
    Vector2& operator*= (Float k);
    Vector2& operator/= (Float k);
    
    Vector2 operator+ (const Vector2 &v) const;
    Vector2 operator- (const Vector2 &v) const;
    Vector2 operator* (Float k) const;
    Vector2 operator/ (Float k) const;
    
    Float norm () const;
    Float normSq () const;
    Vector2& normalize ();
    
    Vector2& offset (Float ox, Float oy);
    Vector2& offsetV (const Vector2 &v, Float d);
    Vector2& offsetVN (const Vector2 &v, Float d);
    Vector2 reverse () const;
  };
  
  class GE_API_ENTRY Vector3
  {
  public:
    Float x, y, z;
    
    Vector2 xy () const;
    Vector4 xyz (Float w) const;
    
    Vector3 ();
    Vector3 (Float x0, Float y0, Float z0);
    Vector3& set (Float x0, Float y0, Float z0);
    Vector3& operator= (const Vector3 &v);
    bool operator== (const Vector3 &v) const;
    bool operator!= (const Vector3 &v) const;
    
    Vector3& operator+= (const Vector3 &v);
    Vector3& operator-= (const Vector3 &v);
    Vector3& operator*= (Float k);
    Vector3& operator/= (Float k);
    
    Vector3 operator+ (const Vector3 &v) const;
    Vector3 operator- (const Vector3 &v) const;
    Vector3 operator* (Float k) const;
    Vector3 operator/ (Float k) const;
    
    Float norm () const;
    Float normSq () const;
    Vector3& normalize ();
    
    Vector3& offset (Float ox, Float oy, Float oz);
    Vector3& offsetV (const Vector3 &v, Float d);
    Vector3& offsetVN (const Vector3 &v, Float d);
    Vector3 reverse () const;
  };
  
  class GE_API_ENTRY Vector4
  {
  public:
    Float x, y, z, w;
    
    Vector2 xy () const;
    Vector3 xyz () const;
    
    Vector4 ();
    Vector4 (Float x0, Float y0, Float z0, Float w0);
    Vector4& set (Float x0, Float y0, Float z0, Float w0);
    Vector4& operator= (const Vector4 &v);
    bool operator== (const Vector4 &v) const;
    bool operator!= (const Vector4 &v) const;
    Vector4& operator+= (const Vector4 &v);
    Vector4& operator-= (const Vector4 &v);
    Vector4& operator*= (Float k);
    Vector4& operator/= (Float k);
    Vector4 operator+ (const Vector4 &v) const;
    Vector4 operator- (const Vector4 &v) const;
    Vector4 operator* (Float k) const;
    Vector4 operator/ (Float k) const;
    
    Float norm () const;
    Float normSq () const;
    Vector4& normalize ();
    
    Vector4& offset (Float ox, Float oy, Float ow, Float oz);
    Vector4& offsetV (const Vector4 &v, Float d);
    Vector4& offsetVN (const Vector4 &v, Float d);
    Vector4 reverse () const;
  };
  
  class GE_API_ENTRY Vector
  {
  public:
    static Float Dot (const Vector2 &v1, const Vector2 &v2);
    static Float Dot (const Vector3 &v1, const Vector3 &v2);
    static Float Dot (const Vector4 &v1, const Vector4 &v2);
    
    static Float Angle (const Vector2 &v1, const Vector2 &v2);
    static Float Angle (const Vector3 &v1, const Vector3 &v2);
    static Float Angle (const Vector4 &v1, const Vector4 &v2);
    
    static Float AngleN (const Vector2 &v1, const Vector2 &v2);
    static Float AngleN (const Vector3 &v1, const Vector3 &v2);
    static Float AngleN (const Vector4 &v1, const Vector4 &v2);
    
    static Float Cross (const Vector2 &v1, const Vector2 &v2);
    static Vector3 Cross (const Vector3 &v1, const Vector3 &v2);

    template <class V> static V Lerp (const V &v1, const V &v2, Float t);
    
    static Float RotationOnPlane2 (const Vector2 &v);
    static Float RotationOnPlane2N (const Vector2 &v);
    static Float RotationOnPlane3 (const Vector3 &v, const Vector3 &ux, const Vector3 &uy);
    static Float RotationOnPlane3N (const Vector3 &v, const Vector3 &ux, const Vector3 &uy);
    
    static Float AreaSign2 (const Vector2 &p1, const Vector2 &p2, const Vector2 &p3);
    static Float AreaSign3 (const Vector3 &p1, const Vector3 &p2, const Vector3 &p3);
    static Float Area2 (const Vector2 &p1, const Vector2 &p2, const Vector2 &p3);
    static Float Area2V (const Vector2 &v1, const Vector2 &v2);
    static Float Area3 (const Vector3 &p1, const Vector3 &p2, const Vector3 &p3);
    static Float Area3V (const Vector3 &v1, const Vector3 &v2);
    static bool InsideTriangle (const Vector2 &p, const Vector2 &p1, const Vector2 &p2,
                                const Vector2 &p3, Float maxerror=0.0001);
  };
  
  /*
  --------------------------------------------
  Info about vertices
  --------------------------------------------
  */
  
  void getVerticesBounds(Vector2 *verts, int size, Vector2 *min, Vector2 *max);

  /*
  =============================================
  Quaternion
  =============================================*/

  class GE_API_ENTRY Quat
  {
  public:
    Float x,y,z,w;

    Quat ();
    Quat (Float x, Float y, Float z, Float w);
    void set (Float x, Float y, Float z, Float w);

    Float norm () const;
    Float normSq () const;
    Quat& normalize ();

    void setIdentity ();
    void fromAxisAngle (Float x, Float y, Float z, Float radang);
    void fromAxisAngle (const Vector3 &axis, Float radang);
    void fromMatrix (const Matrix4x4 &m);
    Matrix4x4 toMatrix ();
    
    static Quat Slerp (const Quat &q1, const Quat &q2, Float t);
    static Quat Nlerp (const Quat &q1, const Quat &q2, Float t);
    inline static Float Dot (const Quat &q1, const Quat &q2);
  };

  Quat GE_API_ENTRY operator* (const Quat &q1, const Quat &q2);

  /*
  ---------------------------------------------
  Unhandled from old code
  ---------------------------------------------

  void Quaternion2Euler(Float q[4], Float *euler);
  Float Quaternion2AxisAngle(Float q[4], Float *axis);
  void ConjugateQuaternion(Float *q);
  void InvertQuaternion(Float *q);
  void MulQuaternions(Float a[4], Float b[4], Float *c);
  void DivQuaternions(Float q1[4], Float q2[4], Float *q);
  */

  
  /*
  =============================================
  Intersections
  =============================================*/
  
  class GE_API_ENTRY Intersection
  {
  public:
    static Vector2 LineLine (const Vector2 &o1, const Vector2 &v1,
                             const Vector2 &o2, const Vector2 &v2);

    static void EllipseEllipse (const Vector2 &c1, const Vector2 &c2,
                                Float rx, Float ry,
                                Vector2 *i1, Vector2 *i2);
  };
  
  /*
  =============================================
  Vector inline functions
  =============================================*/
  
  inline Vector3 Vector2::xy (Float z) const
    { return Vector3 (x,y,z); }
  
  inline Vector4 Vector2::xy (Float z, Float w) const
    { return Vector4 (x,y,z,w); }
  
  
  inline Vector2 Vector3::xy () const
    { return Vector2 (x,y); }
  
  inline Vector4 Vector3::xyz (Float w) const
    { return Vector4 (x,y,z,w); }
  
  
  inline Vector2 Vector4::xy () const
    { return Vector2 (x,y); }
  
  inline Vector3 Vector4::xyz () const
    { return Vector3(x,y,z); }
  
  
  inline Vector2::Vector2 ()
    { x=0.0f; y=0.0f; }
  
  inline Vector3::Vector3 ()
    { x=0.0f; y=0.0f; z=0.0f; }
  
  inline Vector4::Vector4 ()
    { x=0.0f; y=0.0f; z=0.0f; w=0.0f; }
  
  
  inline Vector2::Vector2 (Float x0, Float y0)
    { x=x0; y=y0; }
  
  inline Vector3::Vector3 (Float x0, Float y0, Float z0)
    { x=x0; y=y0; z=z0; }
  
  inline Vector4::Vector4 (Float x0, Float y0, Float z0, Float w0)
    { x=x0; y=y0; z=z0; w=w0; }
  
  
  inline Vector2& Vector2::set (Float x0, Float y0)
    { x=x0; y=y0; return *this; }
  
  inline Vector3& Vector3::set (Float x0, Float y0, Float z0)
    { x=x0; y=y0; z=z0; return *this; }
  
  inline Vector4& Vector4::set (Float x0, Float y0, Float z0, Float w0)
    { x=x0; y=y0; z=z0; w=w0; return *this; }
  
  
  inline Vector2& Vector2::operator= (const Vector2 &v)
    { x=v.x; y=v.y; return *this; }
  
  inline Vector3& Vector3::operator= (const Vector3 &v)
    { x=v.x; y=v.y; z=v.z; return *this; }
  
  inline Vector4& Vector4::operator= (const Vector4 &v)
    { x=v.x; y=v.y; z=v.z; w=v.w; return *this; }
  
  
  inline bool Vector2::operator== (const Vector2 &v) const
    { return x==v.x && y==v.y; }
  
  inline bool Vector3::operator== (const Vector3 &v) const
    { return x==v.x && y==v.y && z==v.z; }
  
  inline bool Vector4::operator== (const Vector4 &v) const
    { return x==v.x && y==v.y && z==v.z && w==v.w; }
  
  
  inline bool Vector2::operator!= (const Vector2 &v) const
    { return x!=v.x || y!=v.y; }
  
  inline bool Vector3::operator!= (const Vector3 &v) const
    { return x!=v.x || y!=v.y || z!=v.z; }
  
  inline bool Vector4::operator!= (const Vector4 &v) const
    { return x!=v.x || y!=v.y || z!=v.z || w!=v.w; }
  
  
  inline Vector2& Vector2::operator+= (const Vector2 &v)
    { x+=v.x; y+=v.y; return *this; }
  
  inline Vector3& Vector3::operator+= (const Vector3 &v)
    { x+=v.x; y+=v.y; z+=v.z; return *this; }
  
  inline Vector4& Vector4::operator+= (const Vector4 &v)
    { x+=v.x; y+=v.y; z+=v.z; w+=v.w; return *this; }
  
  
  inline Vector2& Vector2::operator-= (const Vector2 &v)
    { x-=v.x; y-=v.y; return *this; }
  
  inline Vector3& Vector3::operator-= (const Vector3 &v)
    { x-=v.x; y-=v.y; z-=v.z; return *this; }
  
  inline Vector4& Vector4::operator-= (const Vector4 &v)
    { x-=v.x; y-=v.y; z-=v.z; w-=v.w; return *this; }
  
  
  inline Vector2& Vector2::operator*= (Float k)
    { x*=k; y*=k; return *this; }
  
  inline Vector3& Vector3::operator*= (Float k)
    { x*=k; y*=k; z*=k; return *this; }
  
  inline Vector4& Vector4::operator*= (Float k)
    { x*=k; y*=k; z*=k; w*=k; return *this; }
  
  
  inline Vector2& Vector2::operator/= (Float k)
    { x/=k; y/=k; return *this; }
  
  inline Vector3& Vector3::operator/= (Float k)
    { x/=k; y/=k; z/=k; return *this; }
  
  inline Vector4& Vector4::operator/= (Float k)
    { x/=k; y/=k; z/=k; w/=k; return *this; }
  
  
  inline Vector2 Vector2::operator+ (const Vector2 &v) const
    { return Vector2 (x+v.x, y+v.y); }
  
  inline Vector3 Vector3::operator+ (const Vector3 &v) const
    { return Vector3 (x+v.x, y+v.y, z+v.z); }
  
  inline Vector4 Vector4::operator+ (const Vector4 &v) const
    { return Vector4 (x+v.x, y+v.y, z+v.z, w+v.w); }
  
  
  inline Vector2 Vector2::operator- (const Vector2 &v) const
    { return Vector2 (x-v.x, y-v.y); }
  
  inline Vector3 Vector3::operator- (const Vector3 &v) const
    { return Vector3 (x-v.x, y-v.y, z-v.z); }
  
  inline Vector4 Vector4::operator- (const Vector4 &v) const
    { return Vector4 (x-v.x, y-v.y, z-v.z, w-v.w); }
  
  
  inline Vector2 Vector2::operator* (Float k) const
    { return Vector2 (x*k, y*k); }
  
  inline Vector3 Vector3::operator* (Float k) const
    { return Vector3 (x*k, y*k, z*k); }
  
  inline Vector4 Vector4::operator* (Float k) const
    { return Vector4 (x*k, y*k, z*k, w*k); }
  
  
  inline Vector2 Vector2::operator/ (Float k) const
    { return Vector2 (x/k, y/k); }
  
  inline Vector3 Vector3::operator/ (Float k) const
    { return Vector3 (x/k, y/k, z/k); }
  
  inline Vector4 Vector4::operator/ (Float k) const
    { return Vector4 (x/k, y/k, z/k, w/k); }
  
  
  inline Float Vector2::norm () const
    { return SQRT (x*x + y*y); }
  
  inline Float Vector3::norm () const
    { return SQRT (x*x + y*y + z*z); }
  
  inline Float Vector4::norm () const
    { return SQRT (x*x + y*y + z*z + w*w); }
  
  
  inline Float Vector2::normSq () const
    { return x*x + y*y; }
  
  inline Float Vector3::normSq () const
    { return x*x + y*y + z*z; }
  
  inline Float Vector4::normSq () const
    { return x*x + y*y + z*z + w*w; }
  
  
  inline Vector2& Vector2::normalize ()
    { Float k=1.0f/norm(); x = k*x; y = k*y; return *this; }
  
  inline Vector3& Vector3::normalize ()
    { Float k=1.0f/norm(); x = k*x; y = k*y; z = k*z; return *this; }
  
  inline Vector4& Vector4::normalize ()
    { Float k=1.0f/norm(); x = k*x; y = k*y; z = k*z; w = k*w; return *this; }
  
  
  inline Vector2& Vector2::offset (Float ox, Float oy)
    { x+=ox; y+=oy; return *this; }
  
  inline Vector3& Vector3::offset (Float ox, Float oy, Float oz)
    { x+=ox; y+=oy; z+=oz; return *this; }
  
  inline Vector4& Vector4::offset (Float ox, Float oy, Float oz, Float ow)
    { x+=ox; y+=oy; z+=oz; w+=ow; return *this; }
  
  
  inline Vector2& Vector2::offsetV (const Vector2 &v, Float d)
    { Float K = d/v.norm(); x += v.x*K; y += v.y*K; return *this; }
  
  inline Vector3& Vector3::offsetV (const Vector3 &v, Float d)
    { Float K = d/v.norm(); x += v.x*K; y += v.y*K; z += v.z*K; return *this; }
  
  inline Vector4& Vector4::offsetV (const Vector4 &v, Float d)
    { Float K = d/v.norm(); x += v.x*K; y += v.y*K; z += v.z*K; w += v.w*K; return *this; }
  
  
  inline Vector2& Vector2::offsetVN (const Vector2 &v, Float d)
    { x += v.x*d; y += v.y*d; return *this; }
  
  inline Vector3& Vector3::offsetVN (const Vector3 &v, Float d)
    { x += v.x*d; y += v.y*d; z += v.z*d; return *this; }
  
  inline Vector4& Vector4::offsetVN (const Vector4 &v, Float d)
    { x += v.x*d; y += v.y*d; z += v.z*d; w += v.w*d; return *this; }
  
  
  inline Vector2 Vector2::reverse () const
    { return Vector2 (-x, -y); }
  
  inline Vector3 Vector3::reverse () const
    { return Vector3 (-x, -y, -z); }
  
  inline Vector4 Vector4::reverse () const
    { return Vector4 (-x, -y, -z, -w); }
  
  
  inline Float Vector::Dot (const Vector2 &v1, const Vector2 &v2)
    { return v1.x*v2.x + v1.y*v2.y; }
  
  inline Float Vector::Dot (const Vector3 &v1, const Vector3 &v2)
    { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }
  
  inline Float Vector::Dot (const Vector4 &v1, const Vector4 &v2)
    { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w; }
  
  
  inline Float Vector::Angle (const Vector2 &v1, const Vector2 &v2)
    { return ACOS (Vector::Dot (v1,v2) / (v1.norm() * v2.norm())); }
  
  inline Float Vector::Angle (const Vector3 &v1, const Vector3 &v2)
    { return ACOS (Vector::Dot (v1,v2) / (v1.norm() * v2.norm())); }
  
  inline Float Vector::Angle (const Vector4 &v1, const Vector4 &v2)
    { return ACOS (Vector::Dot (v1,v2) / (v1.norm() * v2.norm())); }
  
  
  inline Float Vector::AngleN (const Vector2 &v1, const Vector2 &v2)
    { return ACOS (Vector::Dot (v1,v2)); }
  
  inline Float Vector::AngleN (const Vector3 &v1, const Vector3 &v2)
    { return ACOS (Vector::Dot (v1,v2)); }
  
  inline Float Vector::AngleN (const Vector4 &v1, const Vector4 &v2)
    { return ACOS (Vector::Dot (v1,v2)); }
  
  
  inline Float Vector::Cross (const Vector2 &v1, const Vector2 &v2) {
    return v1.x*v2.y - v2.x*v1.y;
  }
  
  inline Vector3 Vector::Cross (const Vector3 &v1, const Vector3 &v2) {
    return Vector3 (v1.y*v2.z - v2.y*v1.z,
                    v2.x*v1.z - v1.x*v2.z,
                    v1.x*v2.y - v2.x*v1.y);
  }

  template <class V> inline V Vector::Lerp (const V &v1, const V &v2, Float t) {
    return v1 * (1.0f - t) + v2 * t;
  }
  
  /*
  =============================================
  Quaternion inline functions
  =============================================*/
  
  inline Quat::Quat ()
    { x=0.0f; y=0.0f; z=0.0f; w=1.0f; }
  
  inline Quat::Quat (Float xx, Float yy, Float zz, Float ww)
    { x=xx; y=yy; z=zz; w=ww; }
  
  inline void Quat::set (Float xx, Float yy, Float zz, Float ww)
    { x=xx; y=yy; z=zz; w=ww; }
  
  inline void Quat::setIdentity ()
    { x=0.0f; y=0.0f; z=0.0f; w=1.0f; }
  
  inline Float Quat::norm () const
    { return SQRT (x*x + y*y + z*z + w*w); }
  
  inline Float Quat::normSq () const
    { return x*x + y*y + z*z + w*w; }
  
  inline Quat& Quat::normalize ()
    { Float n=norm(); x/=n; y/=n; z/=n; w/=n; return *this; }
  
  inline Float Quat::Dot (const Quat &q1, const Quat &q2)
    { return q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w; }
  

}//namespace GE
#endif//__VECTORS_H
