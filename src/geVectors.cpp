#define GE_API_EXPORT
#include "geDefs.h"
#include "geVectors.h"
#include "geMatrix.h"

namespace GE
{	
  /*
  =========================================
  Vectors
  =========================================*/
  
  Float Vector::RotationOnPlane2 (const Vector2 &v)
  {
    Float norm = v.norm();
    Float cosa = v.x/norm;
    Float sina = v.y/norm;
    return sina>=0 ? ACOS(cosa) : 2*PI-ACOS(cosa);
  }
  
  Float Vector::RotationOnPlane2N (const Vector2 &v)
  {
    Float cosa = v.x;
    Float sina = v.y;
    return sina>=0 ? ACOS(cosa) : 2*PI-ACOS(cosa);
  }
  
  Float Vector::RotationOnPlane3 (const Vector3 &v, const Vector3 &ux, const Vector3 &uy)
  {
    Float norm = v.norm();
    Float cosa = Vector::Dot(v, ux)/norm;
    Float sina = Vector::Dot(v, uy)/norm;
    return sina>=0 ? ACOS(cosa) : -ACOS(cosa);
  }
  
  Float Vector::RotationOnPlane3N (const Vector3 &v, const Vector3 &ux, const Vector3 &uy)
  {    
    Float cosa = Vector::Dot(v, ux);
    Float sina = Vector::Dot(v, uy);
    return sina>=0 ? ACOS(cosa) : -ACOS(cosa);
  }
  
  Float Vector::AreaSign2 (const Vector2 &p1, const Vector2 &p2, const Vector2 &p3)
  {
    Vector2 v1(p2.x - p1.x, p2.y - p1.y);
    Vector2 v2(p3.x - p1.x, p3.y - p1.y);
    return Vector::Cross(v1,v2)/2;
  }
  
  Float Vector::AreaSign3 (const Vector3 &p1, const Vector3 &p2, const Vector3 &p3)
  {
    Vector3 v1(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
    Vector3 v2(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);
    return Vector::Cross(v1,v2).norm()/2;
  }
  
  Float Vector::Area2 (const Vector2 &p1, const Vector2 &p2, const Vector2 &p3)
  {
    Vector2 v1(p2.x - p1.x, p2.y - p1.y);
    Vector2 v2(p3.x - p1.x, p3.y - p1.y);
    Float signarea = Vector::Cross(v1,v2)/2;
    return signarea < 0.0f ? -signarea : signarea;
  }
  
  Float Vector::Area2V (const Vector2 &v1, const Vector2 &v2)
  {
    Float signarea = Vector::Cross(v1,v2)/2;
    return signarea < 0.0f ? -signarea : signarea;
  }
  
  Float Vector::Area3 (const Vector3 &p1, const Vector3 &p2, const Vector3 &p3)
  {
    Vector3 v1(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
    Vector3 v2(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);
    Float signarea = Vector::Cross(v1,v2).norm()/2;
    return signarea < 0.0f ? -signarea : signarea;
  }
  
  Float Vector::Area3V (const Vector3 &v1, const Vector3 &v2)
  {
    Float signarea = Vector::Cross(v1,v2).norm()/2;
    return signarea < 0.0f ? -signarea : signarea;
  }
  
  bool Vector::InsideTriangle (const Vector2 &p, const Vector2 &p1,
                               const Vector2 &p2, const Vector2 &p3,
                               Float maxerror)
  {
    Float whole = Vector::Area2(p1,p2,p3);
    
    Vector2 v1(p1.x - p.x, p1.y - p.y);
    Vector2 v2(p2.x - p.x, p2.y - p.y);
    Vector2 v3(p3.x - p.x, p3.y - p.y);
    
    return (Vector::Area2V(v1, v2) +
            Vector::Area2V(v2, v3) +
            Vector::Area2V(v3, v1) <
            whole + maxerror);
  }
  
  /*
  ---------------------------------------------
  Vertices info
  ---------------------------------------------*/
  
  void getVertexBounds (Vector2 *verts, int size, Vector2 *min, Vector2 *max)
  {
    min->x = max->x = verts[0].x;
    min->y = max->y = verts[0].y;
    
    for (int l=1; l<size; l++) {
      Vector2 &v = verts[l];
      if (v.x < min->x) min->x = v.x;
      if (v.x > max->x) max->x = v.x;
      if (v.y < min->y) min->y = v.y;
      if (v.y > max->y) max->y = v.y;
    }
  }
  
  /*
  =========================================
  Quaternion
  =========================================*/
  
  void Quat::fromAxisAngle (Float xx, Float yy, Float zz, Float radang)
  {
    Float halfang = radang * 0.5f;
    Float sinha = SIN (halfang);
    x = xx * sinha;
    y = yy * sinha;
    z = zz * sinha;
    w = COS (halfang);
  }
  
  void Quat::fromAxisAngle (const Vector3 &axis, Float radang)
  {
    fromAxisAngle (axis.x, axis.y, axis.z, radang);
  }
  
  Quat GE_API_ENTRY operator* (const Quat &q1, const Quat &q2)
  {
    Quat out;
    out.x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y;
    out.y = q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x;
    out.z = q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w;
    out.w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
    return out;
  }
  
  /*
  --------------------------------------------------------
  See Matrix::fromQuat for the relation of matrix
  elements with the quaternion components. From that we
  can derive the following relations:

  M(0,1) + M(1,0) = 4xy
  M(2,1) + M(1,2) = 4yz
  M(0,2) + M(2,0) = 4xz
  M(1,2) - M(2,1) = 4wx
  M(2,0) - M(0,2) = 4wy
  M(0,1) - M(1,0) = 4wz

  The sum of the M diagonal is:
  4 * (1 - (q.x*q.x + q.y*q.y + q.z*q.z));

  Quaternion is defined as:
  (x*sinA, y*sinA, z*sinA, cosA)
  
  Since (x,y,z) has to be some unit vector:
  4 * (1 - (q.x*q.x + q.y*q.y + q.z*q.z)) =
  4 * (1 - (sq(x) + sq(y) + sq(z)) * sq(sinA)) =
  4 * (1 - sq(sinA)) =
  4 * (sq(cosA)) =
  4 * w * w
  
  From which we derive:
  w = sqrt (diagonal) / 2
  
  Similarly we can subtract two of the diagonal
  components from the third one instead of taking a sum
  to obtain values for x, y or z.
  --------------------------------------------------------*/
  
  void Quat::fromMatrix (const Matrix4x4 &m)
  {
    Float trace, s, inv2s;
    
    //Check if solving for W is possible
    trace = m.m[0][0] + m.m[1][1] + m.m[2][2] + 1.0f;
    if (trace >= 1.0f)
    {
      s = SQRT (trace);
      inv2s = 0.5f / s;
      this->w = s * 0.5f;
      this->x = (m.m[1][2] - m.m[2][1]) * inv2s;
      this->y = (m.m[2][0] - m.m[0][2]) * inv2s;
      this->z = (m.m[0][1] - m.m[1][0]) * inv2s;
    }
    else
    {
      //Solve for X, Y, Z, depending on which component of the
      //diagonal is the biggest to maximize numberic stability

      if (m.m[0][0] > m.m[1][1] && m.m[0][0] > m.m[2][2]) {
        
        //m00 is the biggest
				s = SQRT (m.m[0][0] - m.m[1][1] - m.m[2][2] + 1.0f);
        inv2s = 0.5f / s;
			  this->w = (m.m[1][2] - m.m[2][1]) * inv2s;
				this->x = s * 0.5f;
        this->y = (m.m[0][1] + m.m[1][0]) * inv2s;
				this->z = (m.m[0][2] + m.m[2][0]) * inv2s;

      }else if (m.m[1][1] > m.m[2][2]) {
        
        //m11 is the biggest
        s = SQRT (m.m[1][1] - m.m[0][0] - m.m[2][2] + 1.0f);
        inv2s = 0.5f / s;
        this->w = (m.m[2][0] - m.m[0][2] ) * inv2s;
        this->x = (m.m[0][1] + m.m[1][0] ) * inv2s;
        this->y = s * 0.5f;
        this->z = (m.m[2][1] + m.m[1][2] ) * inv2s;

      }else {
        
        //m22 is the biggest
        s = SQRT (m.m[2][2] - m.m[0][0] - m.m[1][1] + 1.0f);
        inv2s = 0.5f / s;
        this->w = (m.m[0][1] - m.m[1][0] ) * inv2s;
        this->x = (m.m[0][2] + m.m[2][0] ) * inv2s;
        this->y = (m.m[2][1] + m.m[1][2] ) * inv2s;
        this->z = s * 0.5f;
      }
    }
  }

  Matrix4x4 Quat::toMatrix ()
  {
    Matrix4x4 m;
    m.fromQuat (*this);
    return m;
  }
  
  Quat Quat::Slerp (const Quat &q1, const Quat &q2, Float t)
	{
    Float angle = ACOS (Quat::Dot (q1, q2));
    Float sina = SIN (angle);
    Float a = SIN ((1-t) * angle) / sina;
    Float b = SIN (  t   * angle) / sina;

    Quat qout;
    qout.x =  a * q1.x  +  b * q2.x;
    qout.y =  a * q1.y  +  b * q2.y;
    qout.z =  a * q1.z  +  b * q2.z;
    qout.w =  a * q1.w  +  b * q2.w;
    return qout;
	}
  
  /*
  =========================================
  Intersections
  =========================================*/
  
  Vector2 Intersection::LineLine(const Vector2 &o1, const Vector2 &v1, const Vector2 &o2, const Vector2 &v2)
  {
    // This is what the matrices would
    // look like, but we optimize it by
    // not storing before calculating.
    //Float matrixU[2] = {v1.x, -v2.x};
    //Float matrixD[2] = {v1.y, -v2.y};
    //Float matrixU1[2] = {rightU, -v2.x};
    //Float matrixD1[2] = {rightD, -v2.y};
    //Float matrixU2[2] = {v1.x, rightU};
    //Float matrixD2[2] = {v1.y, rightD};
    
    Float rightU = o2.x - o1.x;
    Float rightD = o2.y - o1.y;
    
    Float D  = v1.x   * (-v2.y) - v1.y   * (-v2.x);
    Float DX = rightU * (-v2.y) - rightD * (-v2.x);
  //Float DY = v1.x   * rightD  - v1.y   * rightU;
    
    Float t1 = DX / D;
    return Vector2 (o1.x + t1*v1.x,
                    o1.y + t1*v1.y);
  }
  
  /*
  void Intersection::EllipseEllipse(const Vector2 &c1, const Vector2 &c2, Float rx, Float ry,
                                    Vector2 *i1, Vector2 *i2)
  {
    //precalculations
    Float x1=c1.x, y1=c1.y;
    Float x2=c2.x, y2=c2.y;
    Float rxSq = rx*rx;
    Float rySq = ry*ry;
    Float rxSqInv = 1.0f/rxSq;
    Float rySqInv = 1.0f/rySq;
    Float x1Sq = c1.x*c1.x;
    Float y1Sq = c1.y*c1.y;
    Float x2Sq = c2.x*c2.x;
    Float y2Sq = c2.y*c2.y;
    Float ix1, iy1, ix2, iy2;

    //merge of two ellipse equations
    //with centers at v1 and v2
    Float k1 = rxSqInv * 2*(x2 - x1); //coeff of X
    Float k2 = rySqInv * 2*(y2 - y1); //coeff of Y
    Float k3 = rxSqInv * (x1Sq - x2Sq) + rySqInv * (y1Sq - y2Sq); //free coeff

    if ((int)(k2*10000) == 0) {

      //vertical line
      ix1 = ix2 = (x1 + x2) / 2;

    }else{

      //we get a linear equation
      Float kx = -k1/k2, nx = -k3/k2;
      Float dX = y1 - nx;

      //let's plug it into the first ellipse equation
      //to find the line-ellipse intersection:
      Float ax =         rxSqInv +    kx*kx * rySqInv;     //coeff of X^2
      Float bx = -2*x1 * rxSqInv + -2*kx*dX * rySqInv;     //coeff of X
      Float cx =  x1Sq * rxSqInv +    dX*dX * rySqInv - 1; //free coeff
      
      //solution of quadratic equation
      Float Dx = bx*bx - 4*ax*cx;
			
      if (Dx <= 0.0f) {
        ix1 = ix2 = (x1 + x2) / 2;
      }else{
	      ix1 = ( -bx + SQRT(Dx) )/(2*ax);
  	    ix2 = ( -bx - SQRT(Dx) )/(2*ax);
      }
    }
		
    if ((int)(k1*10000) == 0){

      //horizontal line
      iy1 = iy2 = (y1 + y2) / 2;

    }else{

      //we get a linear equation
      Float ky = -k2/k1, ny = -k3/k1;
      Float dY = x1 - ny;

      //let's plug it into the first ellipse equation
      //to find the line-ellipse intersection:
      Float ay =    ky*ky * rxSqInv +         rySqInv;     //coeff of Y^2
      Float by = -2*ky*dY * rxSqInv + -2*y1 * rySqInv;     //coeff of Y
      Float cy =    dY*dY * rxSqInv +  y1Sq * rySqInv - 1; //free coeff
      
      //solution of quadratic equation
      Float Dy = by*by - 4*ay*cy;

      if (Dy <= 0.0f) {
        iy1 = iy2 = (y1 + y2) / 2;
      }else{
	      iy1 = ( -by + SQRT(Dy) )/(2*ay);
  	    iy2 = ( -by - SQRT(Dy) )/(2*ay);
      }
    }
    
		//swap solutions properly
    if (c1.x > c2.x) {
    	Float ixt = ix1;
    	ix1 = ix2;
      ix2 = ixt;
    }
    if (c1.y <= c2.y) {
    	Float iyt = iy1;
      iy1 = iy2;
      iy2 = iyt;
    }
    
    //return values
    i1->set(ix1, iy1);
    i2->set(ix2, iy2);
  }*/
  
	//Unhandled from old code
	//////////////////////////////////////////////////////////////
                   
  /*

	void Quaternion2Euler(Float q[4], Float *euler)
	{
		euler[0] = atan2(2*(q[2]*q[3]+q[0]*q[1]), q[0]*q[0]-q[1]*q[1]-q[2]*q[2]+q[3]*q[3]);
		euler[1] = atan2(-2*(q[1]*q[3]-q[0]*q[2]), q[0]*q[0]+q[1]*q[1]-q[2]*q[2]+q[3]*q[3]);
		euler[2] = atan2(2*(q[1]*q[2]+q[0]*q[3]), q[0]*q[0]+q[1]*q[1]-q[2]*q[2]-q[3]*q[3]);

	/*	Float test=-2*(q[1]*q[3]-q[0]*q[2]);
		if (test > 1)
		{
			euler[0] += (euler[0]>=0 ? -PI : PI);
			euler[2] += (euler[2]>=0 ? -PI : PI);
		}


		/*	if (fabs(euler) >  90)
		{
			//singularity at north pole
			euler[0] = 2 * atan2f(q[1], q[0]);
			euler[1] = PI/2;
			euler[2] = 0;
			return;
		}
		if (test < -0.499)
		{
			//singularity at south pole
			euler[0] = -2 * atan2f(q[1],q[0]);
			euler[1] = - PI/2;
			euler[2] = 0;
			return;
		}

		double test = q[1]*q[2] + q[3]*q[0];

		double sqx = q[1]*q[1];
		double sqy = q[2]*q[2];
		double sqz = q[3]*q[3];

		euler[0] = atan2f(2*(q[1]*q[0]-q[2]*q[3]), 1-2*sqx-2*sqz);    
		euler[1] = atan2f(2*(q[2]*q[0]-q[1]*q[3]), 1-2*sqy-2*sqz);
		euler[2] = asinf(2*test);
	}*/
  
  /*
	Float Quaternion2AxisAngle(Float q[4], Float *axis)
	{
		Float norm = NORM3((&q[1]));

		axis[0] = q[1] * norm;
		axis[1] = q[2] * norm;
		axis[2] = q[3] * norm;
		
		return 2*ACOS(q[0]);;
	}

	void ConjugateQuaternion(Float *q)
	{
		q[1] = -q[1];
		q[2] = -q[2];
		q[3] = -q[3];
	}

	void InvertQuaternion(Float *q)
	{
		ConjugateQuaternion(q);
	}

	void DivQuaternions(Float q1[4], Float q2[4], Float *q)
	{
		Float q2i[4];
		CopyPoint4(q2, q2i);
		InvertQuaternion(q2i);

		MulQuaternions(q1, q2i, q);
	}
  */
}
