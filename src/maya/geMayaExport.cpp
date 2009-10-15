#include "geMaya.h"
#include <maya/MFileObject.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnLight.h>
#include <maya/MFnSpotLight.h>
#include <maya/MFnPointLight.h>
#include <maya/MFnNonExtendedLight.h>

/*
----------------------------------------------------------
If we convert points for output so that:

  pout = change * pin

applying a transformation T to an outputted point
would yield the same result as applying it before if:

  pT = change * T * inv(change) * change * pin
  pT = change * T * pin

Also, Maya matrices are transposed compared to GameEngine
as Maya treats points as row vectors and transforms them
by multiplying with the matrix on the right side (P * M)
----------------------------------------------------------*/

Matrix4x4 exportMatrix (const MMatrix &m)
{
  Matrix4x4 outm;

  //Get elements as floats
  //(Maya uses doubles internally)
  Float rows[4][4];
  m.get( rows );
  
  //Get rows
  Float *X = rows[0];
  Float *Y = rows[1];
  Float *Z = rows[2];
  Float *T = rows[3];

  //Transpose into columns
  outm.set (X[0], Y[0], Z[0], T[0],
            X[1], Y[1], Z[1], T[1],
            X[2], Y[2], Z[2], T[2],
            0,    0,    0,    1);
  
  //System change matrix
  GE::Matrix4x4 change;
  change.set (1, 0, 0, 0,
              0, 1, 0, 0,
              0, 0,-1, 0,
              0, 0, 0, 1);
  
  return change * outm * change.affineInverse();
}

/*
-------------------------------------------------------
We could apply the change matrix to points as well
but this is just way faster.
-------------------------------------------------------*/

Vector3 exportPoint (const MFloatPoint &p)
{
  return Vector3( (Float) p.x, (Float) p.y, (Float) -p.z );
}

Vector3 exportVector (const MVector &v)
{
  return Vector3( (Float) v.x, (Float) v.y, (Float) -v.z );
}

Vector2 exportUV (double u, double v)
{
  return Vector2( (Float) u, (Float) (1.0 - v) );
}

Quat exportQuat (const MQuaternion &q)
{
  return Quat( (Float) q.x, (Float) q.y, (Float) -q.z, (Float) -q.w );
}

Matrix4x4 exportScale (double s[3])
{
  MMatrix m = MMatrix::identity;
  m.matrix[0][0] = s[0];
  m.matrix[1][1] = s[1];
  m.matrix[2][2] = s[2];
  return exportMatrix( m );
}

Vector3 exportColor (const MColor &c)
{
  return Vector3( c.r, c.g, c.b );
}

/*
----------------------------------------------------
These functions decompose a transformation matrix
or the transform properties of a kJoint node into
rotation, translation and scale components.
It's funny how rotation and rotationOrientation
mean exactly the oposite thing in the composition
of the two transformation types, meaning they need
to be multiplied in different order.
----------------------------------------------------*/

void exportTransformMatrix (const MMatrix &m, Quat &R, Vector3 &T, Matrix4x4 &S)
{
  MTransformationMatrix tm( m );

  MQuaternion jointO = tm.rotationOrientation();
  MQuaternion jointR = tm.rotation();
  MVector jointT = tm.translation( MSpace::kTransform );
  
  R = exportQuat( jointO * jointR );
  T = exportVector( jointT );
  S.setIdentity();

  /*
  double scale[3] = {0.0,0.0,0.0};
  tm.getScale( scale, MSpace::kTransform );
  if (fabs( scale[0] - 1.0 ) > 0.0001 ||
      fabs( scale[1] - 1.0 ) > 0.0001 ||
      fabs( scale[2] - 1.0 ) > 0.0001)
    int zomg = 0;
    */
}

void exportTransformJoint (const MObject &o, Quat &R, Vector3 &T, Matrix4x4 &S)
{
  MFnIkJoint joint( o );
  MQuaternion jointSO, jointR, jointO;
  MVector jointT;
  
  joint.getScaleOrientation( jointSO );
  joint.getRotation( jointR );
  joint.getOrientation( jointO );
  jointT = joint.getTranslation( MSpace::kTransform );

  R = exportQuat( jointSO * jointR * jointO );
  T = exportVector( jointT );
  S.setIdentity();

  /*
  double scale[3] = {0.0,0.0,0.0};
  joint.getScale( scale );
  if (fabs( scale[0] - 1.0 ) > 0.0001 ||
      fabs( scale[1] - 1.0 ) > 0.0001 ||
      fabs( scale[2] - 1.0 ) > 0.0001)
    int zomg = 0;
  */
}

/*
------------------------------------------------
Exports the material of a mesh node
------------------------------------------------*/

bool findTextureFileName (MPlug &plug, CharString &outFileName)
{
  MStatus status;

  //Walk the dep graph upstream and search for file textures
  MItDependencyGraph depIt( plug, MFn::kFileTexture, MItDependencyGraph::kUpstream );
  for ( ; !depIt.isDone(); depIt.next())
  {
    //Find the "fileTextureName" plug on the node
    MFnDependencyNode depNode( depIt.thisNode() );
    MPlug fileNamePlug = depNode.findPlug( "fileTextureName", &status);
    if (status == MStatus::kSuccess)
    {
      //Get string value
      MString fileName;
      fileNamePlug.getValue( fileName );
      CharString p = fileName.asChar();

      //Resolve file name
      MFileObject fo;
      fo.setResolveMethod( MFileObject::kExact );
      fo.setFullName( fileName );
      CharString resolvedFileName = fo.resolvedFullName().asChar();

      //Get relation to project folder
      File file( resolvedFileName );
      outFileName = file.getRelationTo( getProjectFolder(), true );
      return true;
    }
  }

  return false;
}

Material* exportShader (const MObject &shader, bool *hasNormalMap)
{
  MStatus status;

  StandardMaterial *mat;
  bool diffTexUsed = false;
  CharString diffTexName;
  bool normTexUsed = false;
  CharString normTexName;

  //Find the "surfaceShader" plug on this node
  MFnDependencyNode shaderDepNode( shader );
  MPlug surfShaderPlug = shaderDepNode.findPlug( "surfaceShader", &status );
  if (status != MStatus::kSuccess) {
    trace( "Could not find 'surfaceShader' plug on shader node!" );
    return NULL;
  }

  //Get all the surface shaders connected to this plug as destination
  MPlugArray surfShaderSrcPlugs;
  surfShaderPlug.connectedTo( surfShaderSrcPlugs, true, false );
  if (surfShaderSrcPlugs.length() == 0) {
    trace( "No surface shaders connected to shader node!" );
    return NULL;
  }

  //Take the first one
  MObject surfShader = surfShaderSrcPlugs[0].node();
  MFnDependencyNode surfShaderDepNode( surfShader );

  //Only Lambert shading models supported
  if (! surfShader.hasFn( MFn::kLambert )) {
    trace( "Surface shader is not of Lambert  type (unsupported)!" );
    return NULL;
  }

  //Find the "color" plug on the surface shader
  MPlug colorPlug = surfShaderDepNode.findPlug( "color", &status );
  if (status == MStatus::kSuccess)
    if (findTextureFileName( colorPlug, diffTexName ))
      diffTexUsed = true;

  //Find the "normalCamera" plug on the surface shader
  MPlug camNormalPlug = surfShaderDepNode.findPlug( "normalCamera", &status );
  if (status == MStatus::kSuccess)
    if (findTextureFileName( camNormalPlug, normTexName ))
      normTexUsed = true;

  //Create the proper material type
  if (normTexUsed)
    mat = new NormalTexMat;
  else if (diffTexUsed)
    mat = new DiffuseTexMat;
  else
    mat = new StandardMaterial;

  //Export Lambert params
  MFnLambertShader lambert( surfShader );
  mat->setAmbientColor( exportColor( lambert.ambientColor() ));
  mat->setDiffuseColor( exportColor( lambert.color() ));
  mat->setLuminosity( lambert.translucenceCoeff() );

  //Export Phong params
  if (surfShader.hasFn( MFn::kPhong ))
  {
    MFnPhongShader phong( surfShader );
    mat->setSpecularColor( exportColor( phong.specularColor() ));
    mat->setGlossiness( phong.cosPower() / 128.0f );
    mat->setSpecularity( 1.0f );
  }
  else if (surfShader.hasFn( MFn::kBlinn ))
  {
    MFnBlinnShader blinn( surfShader );
    mat->setSpecularColor( exportColor( blinn.specularColor() ) * blinn.specularRollOff() );
    mat->setGlossiness( 1.0f - blinn.eccentricity() * 0.5f );
  }

  //Export diffuse texture
  if (diffTexUsed)
    ((DiffuseTexMat*) mat)->setDiffuseTexture( diffTexName );

  //Export normal texture
  if (normTexUsed) {
    ((NormalTexMat*) mat)->setNormalTexture( normTexName );
    *hasNormalMap = true;
  }

  return mat;
}

Material* exportMaterial (const MDagPath &nodePath, bool *hasNormalMap)
{
  MStatus status;
  bool normalMap = false;
  bool twoSided = false;
  Material *outMaterial = NULL;

  //Check if it's really a mesh
  MFnMesh mesh( nodePath, &status );
  if (status != MStatus::kSuccess)
    return NULL;

  //Find first transform above it
  MFnTransform transform = nodePath.transform();

  //Get the backface culling attribute
  MPlug plugTwoSided = transform.findPlug( "GameTwoSided", false, &status );
  if (status == MStatus::kSuccess)
    plugTwoSided.getValue( twoSided );

  //Get shaders connected to this mesh
  MIntArray materialIDs;
  MObjectArray meshShaders;
  mesh.getConnectedShaders( nodePath.instanceNumber(), meshShaders, materialIDs );
  if (meshShaders.length() == 0) return NULL;

  //Is there more than one shader assigned?
  if (meshShaders.length() > 1)
  {
    //Group them into a multi-material
    MultiMaterial *mm = new MultiMaterial;
    mm->setNumSubMaterials( meshShaders.length() );
    outMaterial = mm;

    //Walk the list of connected shaders
    for (Uint s=0; s<meshShaders.length(); ++s)
    {
      //Export each sub-material
      Material *mat = exportShader( meshShaders[ s ], &normalMap );
      if (mat == NULL) mat = new StandardMaterial();
      mm->setSubMaterial( s, mat );
      
      //Set additional attributes
      ((StandardMaterial*)mat)->setCullBack( !twoSided );
    }
  }
  else
  {
    //Export single material
    Material *mat = exportShader( meshShaders[ 0 ], &normalMap );
    if (mat == NULL) mat = new StandardMaterial();
    outMaterial = mat;

    //Set additional attributes
    ((StandardMaterial*)mat)->setCullBack( !twoSided );
  }

  //Return info on normal map
  if (hasNormalMap != NULL)
    *hasNormalMap = normalMap;

  return outMaterial;
}

Light* exportLight (const MDagPath &lightDagPath)
{
  MStatus status;
  Light *outLight;

  //Check the light type
  if (lightDagPath.hasFn( MFn::kSpotLight ))
  {
    //Find the inner and out angles
    MFnSpotLight spotLight( lightDagPath );
    Float a1 = (Float) spotLight.coneAngle();
    Float a2 = (Float) Util::Max(
      spotLight.coneAngle() * 0.5 +
      spotLight.penumbraAngle(),
      0.0 );

    //Convert to degrees
    a1 = Util::RadToDeg( a1 );
    a2 = Util::RadToDeg( a2 );

    //Set larger angle as outer, smaller as inner
    SpotLight *outSpotLight = new SpotLight;
    outSpotLight->setAngle( Util::Max( a1,a2 ), Util::Min( a1,a2) );
    outLight = outSpotLight;
  }
  else if (lightDagPath.hasFn( MFn::kPointLight ))
  {
    //Create a new point light
    PointLight *outPointLight = new PointLight;
    outLight = outPointLight;
  }
  else return NULL;

  //Get first transform above it
  MFnTransform transform = lightDagPath.transform();

  //Search for custom attenuation attribute plugs
  MStatus plugStatus[2];
  MPlug plugAttStart = transform.findPlug( "AttStart", false, &plugStatus[0] );
  MPlug plugAttEnd = transform.findPlug( "AttEnd", false, &plugStatus[1] );
  if (plugStatus[0] == MStatus::kSuccess &&
      plugStatus[1] == MStatus::kSuccess)
  {
    //Set light attenuation
    Float attStart=0.0f, attEnd=100.0f;
    plugAttStart.getValue( attStart );
    plugAttEnd.getValue( attEnd );
    outLight->setAttenuation(
      attEnd * getWorldScale(),
      attStart * getWorldScale()
    );
  }

  //Search for custom shadows attribute plug
  MPlug plugShadows = transform.findPlug( "CastShadows", false, &plugStatus[0] );
  if (plugStatus[0] == MStatus::kSuccess)
  {
    //Set light shadows
    bool castShadows = false;
    plugShadows.getValue( castShadows );
    outLight->setCastShadows( castShadows );
  }

  //Set other light properties
  MFnLight light( lightDagPath );
  outLight->setDiffuseColor( exportColor( light.color() ) * light.intensity() );
  outLight->setMatrix( exportMatrix( lightDagPath.inclusiveMatrix() ));
  return outLight;
}

DofParams exportDofParams (Float dofNear, Float dofStart, Float dofEnd, Float dofFar)
{
  DofParams dofParams;
  dofParams.focusCenter = ((dofEnd + dofStart) / 2) * getWorldScale();
  dofParams.focusRange = ((dofEnd - dofStart) / 2) * getWorldScale();
  dofParams.falloffNear = (dofStart - dofNear) * getWorldScale();
  dofParams.falloffFar = (dofFar - dofEnd) * getWorldScale();
  return dofParams;
}

Camera* exportCamera (const MDagPath &camDagPath)
{
  //Export camera position
  Camera3D *outCam = new Camera3D;
  outCam->setMatrix( exportMatrix( camDagPath.inclusiveMatrix() ));

  //Export camera attributes
  MFnCamera cam( camDagPath );
  outCam->setNearClipPlane( (Float) cam.nearClippingPlane() * getWorldScale() );
  outCam->setFarClipPlane( (Float) cam.farClippingPlane() * getWorldScale() );

  //Get first transform above it
  MFnTransform transform = camDagPath.transform();

  //Search for custom DOF attribute plugs
  MStatus dofStatus[5];
  MPlug plugDofNear    = transform.findPlug( "DofNear",    false, &dofStatus[0] );
  MPlug plugDofStart   = transform.findPlug( "DofStart",   false, &dofStatus[1] );
  MPlug plugDofEnd     = transform.findPlug( "DofEnd",     false, &dofStatus[2] );
  MPlug plugDofFar     = transform.findPlug( "DofFar",     false, &dofStatus[3] );
  MPlug plugDofEnabled = transform.findPlug( "DofEnabled", false, &dofStatus[4] );
  if (dofStatus[0] == MStatus::kSuccess &&
      dofStatus[1] == MStatus::kSuccess &&
      dofStatus[2] == MStatus::kSuccess &&
      dofStatus[3] == MStatus::kSuccess &&
      dofStatus[4] == MStatus::kSuccess)
  {
    //Get values off the plugs
    bool dofEnabled = false;
    Float dofNear=0.0f, dofStart=0.0f, dofEnd=0.0f, dofFar=0.0f;
    plugDofNear.getValue( dofNear );
    plugDofStart.getValue( dofStart );
    plugDofEnd.getValue( dofEnd );
    plugDofFar.getValue( dofFar );
    plugDofEnabled.getValue( dofEnabled );

    //Set camera dof params
    DofParams dofParams = exportDofParams( dofNear, dofStart, dofEnd, dofFar );
    outCam->setDofParams( dofParams );
    outCam->setDofEnabled( dofEnabled );
  }

  return outCam;
}


/*
-----------------------------------------------
Exports the mesh geometry and UV data.
-----------------------------------------------*/

class MeshExporter
{
private:
  MFloatPointArray meshPoints;
  MFloatVectorArray meshNormals;
  MFloatVectorArray meshTangents;
  MFloatVectorArray meshBitangents;
  MFloatArray meshPointsU;
  MFloatArray meshPointsV;

protected:
  MFloatPoint getPoint (int vertId) {return meshPoints[ vertId ]; }
  MVector getNormal (int normalId) { return meshNormals[ normalId ]; }
  MVector getTangent (int tangentId) { return meshTangents[ tangentId ]; }
  MVector getBitangent (int tangentId) { return meshBitangents[ tangentId ]; }
  double getU (int uvId) { return meshPointsU[ uvId ]; }
  double getV (int uvId) { return meshPointsV[ uvId ]; }

  virtual void visitNumbers (int numPoints, int numNormals, int numTangents, int numUVs, int numFaces, int numMaterials) {}
  virtual void visitPoint (const MFloatPoint &point) {}
  virtual void visitNormal (const MVector &normal) {}
  virtual void visitTangent (const MVector &tangent, const MVector &bitangent) {}
  virtual void visitUV (double u, double v) {}
  virtual void visitPolygonStart (int faceId, int numCorners) {}
  virtual void visitPolygonCorner (int faceId, int corner, int vertId, int normalId, int tangentId, int uvId) {}
  virtual bool visitPolygonEnd (int faceId, int materialId) { return true; }
  virtual bool visitTriangle (int faceId, int materialId, int vertIds[3]) { return true; }
  virtual bool visitEdge (int vertIds[2], bool isSmooth) { return true; }
  virtual void visitReport () {}

public:
  void exportMesh (const MObject &meshNode);
};

void MeshExporter::exportMesh (const MObject &meshNode)
{
  MStatus status;
  MSpace::Space meshPointSpace;
  MIntArray meshNumCorners;
  MIntArray meshFaceVertIds;
  MIntArray meshFaceNormalIds;
  MIntArray meshTriCounts;
  MIntArray meshTriIndices;
  MObjectArray meshShaders;
  MIntArray meshFaceShaderIds;
  Uint meshNumPoints;
  Uint meshNumNormals;
  Uint meshNumTangents;
  Uint meshNumBitangents;
  Uint meshNumPointsUV;
  Uint meshNumFaces;
  Uint meshNumEdges;
  Uint meshNumShaders;

  int invalidVertIds = 0;
  int invalidNormalIds = 0;
  int invalidTangentIds = 0;
  int invalidUVIds = 0;
  int invalidTriangles = 0;
  int invalidEdges = 0;
  int invalidFaces = 0;

  //Get mesh points
  MFnMesh mesh( meshNode, &status );
  meshPointSpace = MSpace::kObject;
  mesh.getPoints( meshPoints, meshPointSpace );
/*
  meshPointSpace = MSpace::kWorld;
  status = mesh.getPoints( meshPoints, meshPointSpace );
  if (status == MStatus::kInvalidParameter)
  {
    meshPointSpace = MSpace::kPostTransform;
    status = mesh.getPoints( meshPoints, meshPointSpace );
    if (status == MStatus::kInvalidParameter)
    {
      meshPointSpace = MSpace::kObject;
      status = mesh.getPoints( meshPoints, meshPointSpace );
    }}
*/
  //Get mesh normals and tangents
  mesh.getNormals( meshNormals, meshPointSpace );
  mesh.getTangents( meshTangents, meshPointSpace );
  mesh.getBinormals( meshBitangents, meshPointSpace );

  //Get mesh UV points
  mesh.getUVs( meshPointsU, meshPointsV );

  //Get triangulation
  mesh.getTriangles( meshTriCounts, meshTriIndices );

  //Get shaders
  MDagPath meshDagPath;
  MFnDagNode meshDagNode( meshNode );
  meshDagNode.getPath( meshDagPath );
  mesh.getConnectedShaders( meshDagPath.instanceNumber(), meshShaders, meshFaceShaderIds );

  //Numbers
  meshNumPoints = meshPoints.length();
  meshNumNormals = meshNormals.length();
  meshNumTangents = meshTangents.length();
  meshNumBitangents = meshBitangents.length();
  meshNumPointsUV = meshPointsU.length();
  meshNumFaces = mesh.numPolygons();
  meshNumEdges = mesh.numEdges();
  meshNumShaders = Util::Max( (int)meshShaders.length(), 1 );

  //VISIT
  visitNumbers(
    (int) meshNumPoints,
    (int) meshNumNormals,
    (int) meshNumTangents,
    (int) meshNumPointsUV,
    (int) meshNumFaces,
    (int) meshNumShaders );

  //Walk the mesh points
  for (Uint p=0; p<meshNumPoints; ++p)
    visitPoint( meshPoints[ p ] );

  //Walk the mesh normals
  for (Uint n=0; n<meshNumNormals; ++n)
    visitNormal( meshNormals[ n ] );

  //Walk the mesh tangents
  for (Uint t=0; t<meshNumTangents; ++t)
    visitTangent( meshTangents[ t ], meshBitangents[ t ] );

  //Walk the mesh UV points
  for (Uint uv=0; uv<meshNumPointsUV; ++uv)
    visitUV( meshPointsU[ uv ], meshPointsV[ uv ] );

  //Walk the mesh polygons
  for (Uint f=0; f<meshNumFaces; ++f)
  {
    //Get polygon material ID
    int meshFaceMaterialId = 0;
    if (f < meshFaceShaderIds.length()) {
      int meshFaceShaderId = meshFaceShaderIds[ f ];

      //Check if the material ID is in range then assign
      if (meshFaceShaderId >= 0 && meshFaceShaderId < (int)meshNumShaders)
        meshFaceMaterialId = meshFaceShaderId;
    }

    //Get polygon corner indices
    meshFaceVertIds.clear();
    mesh.getPolygonVertices( f, meshFaceVertIds);
    Uint meshNumFaceVerts = meshFaceVertIds.length();

    //Get polygon normal indices
    meshFaceNormalIds.clear();
    mesh.getFaceNormalIds( f, meshFaceNormalIds );
    Uint meshNumFaceNormals = meshFaceNormalIds.length();

    //VISIT
    visitPolygonStart( f, meshNumFaceVerts );

    //Walk the face vertices
    for (Uint c=0; c<meshNumFaceVerts; ++c)
    {
      //Make sure we have the point available
      int vertId = meshFaceVertIds[ c ];
      if ((Uint) vertId >= meshPoints.length()) {
        invalidVertIds++;
        continue; }

      //Make sure we have the normal available
      if (c > meshNumFaceNormals) {
        invalidNormalIds++;
        continue; }

      int normalId = meshFaceNormalIds[ c ];
      if ((Uint) normalId >= meshNormals.length()) {
        invalidNormalIds++;
        continue; }

      //Make sure we have the tangent available
      int tangentId = mesh.getTangentId( f, vertId, &status );
      if (status != MStatus::kSuccess ) {
        invalidTangentIds++;
        continue; }

      if ((Uint) tangentId >= meshTangents.length()) {
        invalidTangentIds++;
        continue; }

      //Make sure we have the UV point available
      int uvId = -1; status = mesh.getPolygonUVid( f, c, uvId );
      if (status != MStatus::kSuccess) {
        invalidUVIds++;
        continue; }

      if ((Uint) uvId >= meshPointsU.length()) {
        invalidUVIds++;
        continue; }

      //VISIT
      visitPolygonCorner( f, c, vertId, normalId, tangentId, uvId );
    }

    //VISIT
    if (!visitPolygonEnd( f, meshFaceMaterialId )) {
      invalidFaces++;
      continue;
    }

    //Walk the polygon triangles
    for (int t=0; t<meshTriCounts[f]; ++t)
    {
      int meshTriVertIds[3];
      bool valid = true;

      //Get triangle corner indices
      mesh.getPolygonTriangleVertices( f, t, meshTriVertIds );

      //Walk the triangle vertices
      for (Uint c=0; c<3; ++c)
      {
        //Make sure we have the vertex available
        int vertId = meshTriVertIds[ c ];
        if ((Uint)vertId >= meshPoints.length()) {
          invalidTriangles++;
          valid = false;
          break; }
      }

      //Must be valid
      if (!valid) continue;

      //VISIT
      if (!visitTriangle( f, meshFaceMaterialId, meshTriVertIds ))
        invalidTriangles++;
    }
  }

  //Walk the mesh edges
  for (Uint e=0; e<meshNumEdges; ++e)
  {
    //Get the edge endpoints
    int2 edgeVertIds;
    mesh.getEdgeVertices( e, edgeVertIds );

    //Make sure we have the vertices available
    if ((Uint) edgeVertIds[0] >= meshPoints.length() ||
        (Uint) edgeVertIds[1] >= meshPoints.length()) {
      invalidEdges++;
      continue; }

    //VISIT
    if (!visitEdge( edgeVertIds, mesh.isEdgeSmooth( e ) ))
      invalidEdges++;
  }

  //Report
  //----------------------------------------------------

  if (invalidVertIds > 0) {
    trace( "exportMesh: " + CharString::FInt( invalidVertIds )
           + " invalid vertex indices encountered!" ); }
  if (invalidNormalIds > 0) {
    trace( "exportMesh: " + CharString::FInt( invalidNormalIds )
           + " invalid normal indices encountered!" ); }
  if (invalidTangentIds > 0) {
    trace( "exportMesh: " + CharString::FInt( invalidTangentIds )
           + " invalid tangent indices encountered!" ); }
  if (invalidUVIds > 0 ) {
    trace( "exportMesh: " + CharString::FInt( invalidUVIds )
           + " invalid UV vertex indices encountered!" ); }
  if (invalidFaces > 0) {
    trace( "exportMesh: " + CharString::FInt( invalidFaces )
           + " invalid faces encountered!" ); }
  if (invalidTriangles > 0) {
    trace( "exportMesh: " + CharString::FInt( invalidTriangles )
           + " invalid triangles encountered!" ); }
  if (invalidEdges > 0) {
    trace( "exportMesh: " + CharString::FInt( invalidEdges )
           + " invalid edges encountered!" ); }

  //VISIT
  visitReport();
}

/*
-----------------------------------------------------------
Mesh exporter for TriMesh
-----------------------------------------------------------*/

struct Variant
{
  Uint id;
  Uint normalId;
  Uint tangentId;
  Uint uvId;
};

typedef LinkedList<Variant>::Iterator VariantIter;

class VertToVariantMap
{
private:

  bool matchNormal;
  bool matchTangent;
  bool matchUV;

  LinkedList<Variant> variants;
  ArrayList<VariantIter> firstVariants;

public:

  VertToVariantMap ()
  {
    matchNormal = true;
    matchTangent = true;
    matchUV = true;
  }

  void setVariantMask (bool normal, bool tangent, bool uv)
  {
    matchNormal = normal;
    matchTangent = tangent;
    matchUV = uv;
  }

  void clear ()
  {
    variants.clear();
    firstVariants.clear();
  }

  void init (UintSize numVerts)
  {
    clear();

    //Add variant list terminator for each vertex
    for (UintSize v=0; v<numVerts; ++v)
    {
      Variant var;
      var.id = -1;
      VariantIter iter = variants.pushBack( var );
      firstVariants.pushBack( iter );
    }
  }

  UintSize size()
  {
    return firstVariants.size();
  }

  int findVariantId (Uint vertId, Uint normalId, Uint tangentId, Uint uvId)
  {
    //Check if vertex within range
    if (vertId >= firstVariants.size())
      return -1;

    //Walk the variant list until terminator found
    VariantIter var = firstVariants[ vertId ];
    for (; var->id != -1; ++var)
    {
      //Compare
      if (matchNormal) if (var->normalId != normalId) continue;
      if (matchTangent) if (var->tangentId != tangentId) continue;
      if (matchUV) if (var->uvId != uvId) continue;
      return (int) var->id;
    }

    return -1;
  }

  void addVariant (Uint variantId, Uint vertId, Uint normalId, Uint tangentId, Uint uvId)
  {
    if (vertId >= firstVariants.size())
      return;

    Variant newVar;
    newVar.id = variantId;
    newVar.normalId = normalId;
    newVar.tangentId = tangentId;
    newVar.uvId = uvId;

    VariantIter varIt = firstVariants[ vertId ];
    VariantIter newVarIt = variants.insertAt( varIt, newVar );
    firstVariants[ vertId ] = newVarIt;
  }

  VariantIter getFirstVariant (Uint vertId)
  {
    return firstVariants[ vertId ];
  }
};

class OmniVertex : public Object
{
public:
  Vector2 *texcoord;
  Vector3 *coord;
  Vector3 *normal;
  Vector3 *tangent;
  Vector3 *bitangent;
  Float32 *jointWeight;
  Float32 *jointIndex;

  DECLARE_SUBCLASS( OmniVertex, Object );
  DECLARE_MEMBER_DATA( texcoord, new BindTarget( ShaderData::TexCoord2 ) );
  DECLARE_MEMBER_DATA( normal, new BindTarget( ShaderData::Normal ) );
  DECLARE_MEMBER_DATA( coord, new BindTarget( ShaderData::Coord3 ) );
  DECLARE_MEMBER_DATA( tangent, new BindTarget( ShaderData::Tangent ) );
  DECLARE_MEMBER_DATA( bitangent, new BindTarget( ShaderData::Bitangent ) );
  DECLARE_MEMBER_DATA( jointWeight, new BindTarget( ShaderData::JointWeight ) );
  DECLARE_MEMBER_DATA( jointIndex, new BindTarget( ShaderData::JointIndex ) );
  DECLARE_END;
};

DEFINE_CLASS( OmniVertex );

class TriMeshExporter : public MeshExporter
{
protected:

  TriMesh *outTriMesh;
  VertToVariantMap *vertToVariantMap;
  ArrayList<int> faceVertVariantIds;
  ArrayList<int> *indexGroups;
  int numIndexGroups;
  int faceMaxCorners;
  int faceNumCorners;
  VertexBinding <OmniVertex> vertBind;

  virtual void newVertex (
    const Vector3 &point,
    const Vector3 &normal,
    const Vector3 &tangent,
    const Vector3 &bitangent,
    const Vector2 &uv )
  {
    
    OmniVertex vert = vertBind( outTriMesh->addVertex() );
    if (vert.coord != NULL) *vert.coord = point;
    if (vert.normal != NULL) *vert.normal = normal;
    if (vert.tangent != NULL) *vert.tangent = tangent;
    if (vert.bitangent != NULL) *vert.bitangent = bitangent;
    if (vert.texcoord != NULL) *vert.texcoord = uv;
  }

public:

  TriMeshExporter::TriMeshExporter
    (TriMesh *mesh,
     VertToVariantMap &variants)
  {
    outTriMesh = mesh;
    vertToVariantMap = &variants;

    //Init vertex binding
    const VertexFormat *format = mesh->getFormat();
    vertBind.init( format );

    //Init variant matching mask
    FormatMember *fmember = format->findMember( ShaderData::Tangent, "" );
    if (fmember == NULL) vertToVariantMap->setVariantMask( true, false, true );
    else vertToVariantMap->setVariantMask( true, true, true );
  }

  void visitNumbers (int numPoints, int numNormals, int numTangents, int numUVs, int numFaces, int numMaterials)
  {
    //Init vertex to variant map
    vertToVariantMap->init( numPoints );
    for (int p=0; p<numPoints; ++p)
      faceVertVariantIds.pushBack( -1 );

    //Init array of per-group index arrays
    indexGroups = new ArrayList<int>[ numMaterials ];
    numIndexGroups = numMaterials;
  }

  void visitPolygonStart (int faceId, int numCorners)
  {
    //Store the required number of corners
    faceMaxCorners = numCorners;
    faceNumCorners = 0;
  }

  void visitPolygonCorner (int faceId, int corner, int vertId, int normalId, int tangentId, int uvId)
  {
    //Increase the number of valid corners
    faceNumCorners += 1;

    //Find existing vertex variant for these indices
    int variantId = vertToVariantMap->findVariantId(
      vertId, normalId, tangentId, uvId );

    //Use existing variant ID if found
    if (variantId != -1) {
      faceVertVariantIds[ vertId ] = variantId;
      return;
    }

    //Add new vertex to mesh
    newVertex(
      exportPoint( getPoint( vertId ) ),
      exportVector( getNormal( normalId ) ),
      exportVector( getTangent( tangentId ) ),
      exportVector( getBitangent( tangentId ) ),
      exportUV( getU( uvId ), getV( uvId ) ));

    //Use new variant ID
    variantId = (int) outTriMesh->getVertexCount()-1;
    faceVertVariantIds[ vertId ] = variantId;

    //Add new variant ID to map
    vertToVariantMap->addVariant( variantId,
      vertId, normalId, tangentId, uvId);
  }

  bool visitTriangle (int faceId, int materialId, int vertIds[3])
  {
    //Make sure we have all the vertex variants available
    if (faceNumCorners != faceMaxCorners)
      return false;

    //Add new triangle to group
    ArrayList<int> &group = indexGroups[ materialId ];
    group.pushBack( faceVertVariantIds[ vertIds[0] ] );
    group.pushBack( faceVertVariantIds[ vertIds[1] ] );
    group.pushBack( faceVertVariantIds[ vertIds[2] ] );

    return true;
  }

  void visitReport ()
  {
    //Walk the index groups
    for (int g=0; g<numIndexGroups; ++g)
    {
      //Create a face group for this material
      ArrayList<int> &group = indexGroups[ g ];
      outTriMesh->addFaceGroup( (MaterialID) g );

      //Walk the triples of indices in this group
      for (UintSize i=0; i<group.size(); i+=3)
      {
        //Add new triangle to mesh
        outTriMesh->addFace(
          (VertexID) group[ i+0 ],
          (VertexID) group[ i+1 ],
          (VertexID) group[ i+2 ] );
      }
    }

    //Cleanup
    delete[] indexGroups;

    //Report
    trace( "exportMesh: exported "
           + CharString::FInt( (int)outTriMesh->getVertexCount() ) + " vertices." );
    trace( "exportMesh: exported "
           + CharString::FInt( (int)outTriMesh->getFaceCount() ) + " faces." );
  }
};

/*
-----------------------------------------------------------
Mesh exporter for PolyMesh
-----------------------------------------------------------*/

class PolyMeshExporter : public MeshExporter
{
  SPolyMesh *outPolyMesh;
  TexMesh *outTexMesh;
  ArrayList<SPolyMesh::Vertex*> outVerts;
  ArrayList<SPolyMesh::Vertex*> outFaceVerts;
  ArrayList<TexMesh::Vertex*> outTexVerts;
  ArrayList<TexMesh::Vertex*> outTexFaceVerts;
  SPolyMesh::Face *lastPolyFace;
  TexMesh::Face *lastTexFace;

public:

  PolyMeshExporter( SPolyMesh *polyMesh, TexMesh *texMesh ) {
    outPolyMesh = polyMesh;
    outTexMesh = texMesh;
  }

  void visitNumbers (int numPoints, int numNormals, int numTangents, int numUVs, int numFaces, int numMaterials)
  {
    outVerts.reserve( numPoints );
    outTexVerts.reserve( numUVs );
  }

  void visitPoint (const MFloatPoint &point)
  {
    //Add new vertex to the mesh
    SPolyMesh::Vertex *outVert = outPolyMesh->addVertex();
    outVert->point = exportPoint( point );
    outVerts.pushBack( outVert );
  }

  void visitUV (double u, double v)
  {
    //Add new vertex to the texture mesh
    TexMesh::Vertex *outTexVert = outTexMesh->addVertex();
    outTexVert->point = exportUV( u, v );
    outTexVerts.pushBack( outTexVert );
  }

  void visitPolygonStart (int faceId, int numCorners)
  {
    //Init the face vertex array
    outFaceVerts.reserve( numCorners );
    outFaceVerts.clear();

    //Init the UV face vertex array
    outTexFaceVerts.reserve( numCorners );
    outTexFaceVerts.clear();
  }

  void visitPolygonCorner (int faceId, int corner, int vertId, int normalId, int tangentId, int uvId)
  {
    //Add vertex to the face array
    outFaceVerts.pushBack( outVerts[ vertId ] );
    outTexFaceVerts.pushBack( outTexVerts[ uvId ] );
  }

  bool visitPolygonEnd (int faceId, int materialId)
  {
    //Add new face to the mesh
    SPolyMesh::Face *face = outPolyMesh->addFace(
      outFaceVerts.buffer(), (int)outFaceVerts.size() );
    if (face == NULL) return false;
    lastPolyFace = face;

    //Add new face to the UV mesh
    TexMesh::Face *texFace = outTexMesh->addFace(
      outTexFaceVerts.buffer(), (int)outTexFaceVerts.size() );
    if (texFace == NULL) return false;
    lastTexFace = texFace;

    return true;
  }

  bool visitTriangle (int faceId, int materialId, int vertIds[3])
  {
    SPolyMesh::HalfEdge *triHedges[3];

    //Walk the triangle vertices
    for (Uint c=0; c<3; ++c)
    {
      //Find the half edge to that vertex in the face
      SPolyMesh::Vertex *vert = outVerts[ vertIds[ c ] ];
      triHedges[ c ] = lastPolyFace->hedgeTo( vert );
      if (triHedges[ c ] == NULL) return false;
    }

    //Add new triangle to the face
    outPolyMesh->addTriangle(
      lastPolyFace,
      triHedges[0],
      triHedges[1],
      triHedges[2] );

    return true;
  }

  bool visitEdge (int vertIds[2], bool isSmooth)
  {
    //Find the edge connecting the vertices
    SPolyMesh::Vertex *vert1 = outVerts[ vertIds[0] ];
    SPolyMesh::Vertex *vert2 = outVerts[ vertIds[1] ];
    SPolyMesh::HalfEdge *hedge = vert1->outHedgeTo( vert2 );
    if (hedge == NULL) return false;
    
    //Assign the smoothness property
    hedge->fullEdge()->isSmooth = isSmooth;
    return true;
  }

  void visitReport ()
  {
    trace( "exportMesh: exported "
           + CharString::FInt( outPolyMesh->vertexCount() ) + " vertices." );
    trace( "exportMesh: exported "
           + CharString::FInt( outPolyMesh->faceCount() ) + " faces." );
    trace( "exportMesh: exported "
           + CharString::FInt( outTexMesh->vertexCount() ) + " UV vertices." );
    trace( "exportMesh: exported "
           + CharString::FInt( outTexMesh->faceCount() ) + " UV faces." );
  }
};

/*
------------------------------------------------------------------
Finds the skin node and skin pose geometry for the given mesh
node. The "inMesh" plug is inspected for a connection to it that
should come from a SkinCluster node.
------------------------------------------------------------------*/

bool findSkinForMesh (const MObject &meshNode, MObject &skinNode)
{
  MStatus status;
  MFnDagNode dagNode( meshNode );
  
  //Find the plug named "inMesh" in the mesh object
  MPlug inMeshPlug = dagNode.findPlug("inMesh", &status );
  if (status != MStatus::kSuccess || !inMeshPlug.isConnected()) {
    //trace( "findSkinForMesh: 'inMesh' plug missing or not connected!" );
    return false; }

  //Walk the dependency graph upstream on plug level
  MItDependencyGraph dgIt(
    inMeshPlug, MFn::kInvalid,
    MItDependencyGraph::kUpstream,
    MItDependencyGraph::kDepthFirst,
    MItDependencyGraph::kPlugLevel,
    &status );

  for (; !dgIt.isDone(); dgIt.next())
  {
    //Check whether the node is a skin
    MObject dgObj = dgIt.thisNode();
    if (dgObj.apiType() == MFn::kSkinClusterFilter)
    {
      //Found it!
      skinNode = dgObj;
      return true;
    }
  }

  //Not found
  //trace( "findSkinForMesh: no skin found on the 'inMesh' plug!" );
  return false;
}

/*
----------------------------------------------------------
Finds the root joint of the skin cluster's influence tree
----------------------------------------------------------*/

bool findSkinJointRoot (const MObject &skinNode, MObject &rootJoint)
{
  //Get the bones influencing the skin
  MFnSkinCluster skin( skinNode );
  MDagPathArray skinInfluencePaths;
  skin.influenceObjects( skinInfluencePaths );

  //Make sure we got at least one
  if (skinInfluencePaths.length() == 0) {
    trace( "findSkinJointRoot: no influences found in the skin cluster!" );
    return false; }

  //Start with the first influence
  MFnDagNode joint( skinInfluencePaths[ 0 ] );

  //Loop until no kJoint parent found
  bool parentFound = true;
  while (parentFound)
  {
    //Walk all parents and search for a kJoint
    parentFound = false;
    for (Uint p=0; p<joint.parentCount(); ++p) {
      if (joint.parent(p).hasFn( MFn::kJoint )) {
        joint.setObject( joint.parent(p) );
        parentFound = true;
        break;
      }}
  }

  //Output object
  rootJoint = joint.object();

  //Report
  trace( "findSkinJointRoot: found root '"
         + CharString(joint.name().asChar()) + "'");

  return true;
}

/*
--------------------------------------------------------------------
The pose geometry is stored in a disconnected plug called "input".
The data is stored there at the time the mesh is bound to the skin.
--------------------------------------------------------------------*/

bool findSkinPoseMesh (const MObject &skinNode, MObject &poseMesh)
{
  MStatus status;
  MFnSkinCluster skin( skinNode );
  
  //Make sure the input plug is available
  MPlug plugInputArray = skin.findPlug( "input", &status );
  if (status != MStatus::kSuccess) {
    trace ( "findPoseMesh: failed finding array plug named 'input'!" );
    return false ; }

  //Take the first component child of the first element plug in the array
  MPlug plugInput = plugInputArray.elementByLogicalIndex(0);
  MPlug plugGeom = plugInput.child(0);
  plugGeom.getValue( poseMesh );
  return true;
}

/*
-----------------------------------------------------------------
Builds the joint hierarchy tree.
-----------------------------------------------------------------*/

void buildJointTree (const MObject &nodeRoot,
                     ArrayList<MObject> &nodeTree,
                     ArrayList<SkinJoint> &jointTree)
{
  //Push root joint onto the queue
  LinkedList< MObject > dagQueue;
  dagQueue.pushBack( nodeRoot );
  while (!dagQueue.empty())
  {
    //Pop a node from the queue
    MObject dagObj = dagQueue.first();
    MFnDagNode dagNode( dagObj );
    dagQueue.popFront();

    //Get the name of the node
    CharString name = dagNode.name().asChar();
    trace("buildJointTree: - " + name);

    //Add the node and a joint to the tree
    nodeTree.pushBack( dagObj );
    jointTree.pushBack( SkinJoint() );

    //Init bone info
    SkinJoint *outJoint = &jointTree.last();
    outJoint->name = name;
    outJoint->numChildren = 0;

    //Walk node children
    for (Uint c=0; c<dagNode.childCount(); ++c)
    {
      //Check if it's a kJoint type
      MObject child = dagNode.child( c );
      if (child.hasFn( MFn::kJoint ))
      {
        //Push onto the queue and add to joint
        dagQueue.pushBack( child );
        outJoint->numChildren++;
      }
    }
  }

  //Report
  trace(
    "buildJointTree: exported " +
    CharString::FInt( (int) jointTree.size() ) + " joints." );
}

/*
----------------------------------------------------------
Builds an array mapping the skin influence indices to the
indices of their respective joint nodes in the node tree.
----------------------------------------------------------*/

void buildSkinToTreeMap (const MObject &skinNode,
                         const ArrayList<MObject> &nodeTree,
                         ArrayList<Uint> &skinToTreeMap)
{
  //Get the bones influencing the skin
  MFnSkinCluster skin( skinNode );
  MDagPathArray skinInfluencePaths;
  skin.influenceObjects( skinInfluencePaths );

  //Walk the list of influences
  for (Uint i=0; i<skinInfluencePaths.length(); ++i)
  {
    //Find the influence in the joint tree
    int treeIndex = nodeTree.indexOf( skinInfluencePaths[i].node() );
    if (treeIndex >= 0)
    {
      //Map skin influence index to joint tree index
      skinToTreeMap.pushBack( treeIndex );
    }
    else
    {
      //Map to 0 if not found
      skinToTreeMap.pushBack( 0 );
      trace( "buildSkinToTreeMap: skin influence #" +
             CharString::FInt( (int) i ) +
             " missing in the tree!" );
    }
  }
}

/*
-----------------------------------------------------------------
Maya stores the pose inverse matrices in an array plug of the
skin node called "bindPreMatrix". This plug is not connected but
the data is stored into it for every influence added to the
skin at the time of binding.

The bindPreMatrix array plug elements are never removed even if
the joint influences are removed from the skin. This means the
index of a joint influence in the current influence array might
not be the same as the index of its matrix in the bindPreMatrix
plug in case any influences were removed in the middle of the
array. The index that relates the two is called "logical index"
and those indices just keep increasing when new bones are added.
------------------------------------------------------------------*/

void exportSkinPose (const MObject &skinNode,
                     const ArrayList<SkinJoint> &jointTree,
                     const ArrayList<Uint> &skinToTreeMap)
{
  MStatus status;
  MMatrix *invMatrices = new MMatrix[ jointTree.size() ];

  //Get the array plug holding bone world-inverse matrices
  MFnSkinCluster skin( skinNode );
  MPlug plugWorldInvArray = skin.findPlug( "bindPreMatrix", &status );
  if (status != MStatus::kSuccess) {
    trace( "exportPose: failed finding array plug named 'bindPreMatrix'!" );
    return; }

  //Get the bones influencing the skin
  MDagPathArray skinInfluencePaths;
  skin.influenceObjects( skinInfluencePaths );

  //Walk the list of influences
  for (Uint i=0; i<skinInfluencePaths.length(); ++i)
  {
    //Get the bindPreMatrix element plug at the logical index of the influence
    int logicalIndex = skin.indexForInfluenceObject( skinInfluencePaths[i] );
    MPlug plugWorldInv = plugWorldInvArray.elementByLogicalIndex( logicalIndex, &status );
    
    //Make sure the plug is valid
    if (status != MStatus::kSuccess) {
      trace( "exportPose: failed finding bindPreMatrix plug for influence #" +
        CharString::FInt( (int)i ));
      continue; }
    
    //Get the matrix value from the element plug
    MObject plugValue; plugWorldInv.getValue( plugValue );
    MFnMatrixData invMatrixFn( plugValue );
    MMatrix invMatrix = invMatrixFn.matrix();

    //Assign the inverse pose matrix to the joint
    SkinJoint *joint = &jointTree[ skinToTreeMap[i] ];
    joint->worldInv = exportMatrix( invMatrix );
    invMatrices[ skinToTreeMap[i] ] = invMatrix;
  }

  //Export root pose
  SkinJoint *root = &jointTree[ 0 ];
  MMatrix rootLocalMatrix = invMatrices[ 0 ].inverse();
  exportTransformMatrix( rootLocalMatrix,
    root->localR, root->localT, root->localS );

  //Walk the joint tree
  for (UintSize p=0, c=1; p<jointTree.size(); ++p)
  {
    //Walk all the children of this joint
    for (Uint32 pc=0; pc<jointTree[p].numChildren; ++pc)
    {
      //Get the local transformation matrix
      MMatrix worldMatrix = invMatrices[ c ].inverse();
      MMatrix localMatrix = worldMatrix * invMatrices[ p ];

      //Export child pose
      SkinJoint *child = &jointTree[ c ];
      exportTransformMatrix( localMatrix,
        child->localR, child->localT, child->localS );

      //Next child
      c++;
    }
  }

  //Cleanup
  delete[] invMatrices;
}

/*
---------------------------------------------------------------
Maya associates weights with bones rather than vertices.
For each influence (bone) a list of "selections" is obtained.
Each selection is represented with a pair of a DagPath to an
object and the list of exact components (e.g. vertices) in
that object being influenced. The weights for each component
for the current influence is returned in an array in the same
order the components are listed within the selection list.
--------------------------------------------------------------*/

class WeightExporter
{
public:
  virtual int getNumVertices () = 0;
  virtual void setWeight (int vertId, int weightId, int jointId, Float weight) = 0;
  void exportSkinWeights (const MObject &skinNode,
                          const MObject &meshNode,
                          const ArrayList<Uint> &skinToTreeMap);
};

void WeightExporter::exportSkinWeights (const MObject &skinNode,
                                        const MObject &meshNode,
                                        const ArrayList<Uint> &skinToTreeMap)
{
  //Get number of vertices
  int *numWeights = NULL;
  int numVerts = getNumVertices();
  if (numVerts <= 0) return;
  numWeights = new int [numVerts];

  //Initialize weights to 0
  for (int v=0; v<numVerts; ++v) {
    numWeights[v] = 0;
    for (int w=0; w<4; ++w)
      setWeight( v, w, 0, 0.0f );
  }

  //Get the bones influencing the skin
  MFnSkinCluster skin( skinNode );
  MDagPathArray skinInfluencePaths;
  skin.influenceObjects( skinInfluencePaths );

  //Walk the list of influences
  for (Uint i=0; i<skinInfluencePaths.length(); ++i)
  {
    //Get selections of points affected by this influence
    MSelectionList selections; MDoubleArray weights;
    skin.getPointsAffectedByInfluence( skinInfluencePaths[i], selections, weights );

    //Walk the selection array
    for (Uint s=0; s<selections.length(); ++s)
    {
      //Get the object path and component list
      MDagPath dagPath; MObject compObj;
      selections.getDagPath( s, dagPath, compObj );

      //Only process components belonging to the given mesh
      if (dagPath.node() != meshNode) continue;
      if (!compObj.hasFn( MFn::kSingleIndexedComponent )) continue;
      MFnSingleIndexedComponent component( compObj );

      //Walk the component list
      for (Int c=0; c<component.elementCount(); ++c)
      {
        //Get vertex index from the component list
        int vertId = component.element( c );
        if (vertId >= numVerts) continue;

        //Check if number of weights reached 4
        if (numWeights[ vertId ] >= 4) {
          numWeights[ vertId ]++;
          continue; }

        //Add weight to the vertex
        setWeight( vertId,
          numWeights[ vertId ],
          skinToTreeMap[ i ],
          (Float) weights[ c ] );

        numWeights[ vertId ]++;
      }
    }
  }

  //Re-check vertices for errors
  Int zero = 0, many = 0;
  for (int v=0; v<numVerts; ++v) {
    if (numWeights[v] == 0) zero++;
    if (numWeights[v] > 4) many++;
  }

  //Cleanup
  delete[] numWeights;

  //Report
  if (zero > 0)
    trace( "exportSkinWeights: " + CharString::FInt( zero )
    + " vertices have no weight!" );

  if (many > 0)
    trace( "exportSkinWeights: " + CharString::FInt( many )
    + " vertices have too many weights!" );
}

/*
---------------------------------------------------------
Weight exporter for PolyMesh
---------------------------------------------------------*/

class PolyWeightExporter : public WeightExporter
{
  ArrayList<SPolyMesh::Vertex*> outVerts;

public:

  PolyWeightExporter (SPolyMesh *mesh) {
    for (SPolyMesh::VertIter v( mesh ); !v.end(); ++v)
      outVerts.pushBack( *v );
  }

  int getNumVertices () {
    return (int) outVerts.size();
  }

  void setWeight (int vertId, int weightId, int jointId, Float weight) {
    SPolyMesh::Vertex *outVert = outVerts[ vertId ];
    outVert->boneIndex[ weightId ] = jointId;
    outVert->boneWeight[ weightId ] = weight;
  }
};

/*
---------------------------------------------------------
Weight exporter for TriMesh
---------------------------------------------------------*/

class TriWeightExporter : public WeightExporter
{
  SkinTriMesh *outTriMesh;
  VertToVariantMap *vertToVariantMap;
  VertexBinding <SkinVertex> vertBind;

public:

  TriWeightExporter (SkinTriMesh *mesh, VertToVariantMap &variants) {
    outTriMesh = mesh;
    vertToVariantMap = &variants;
    vertBind.init( mesh->getFormat() );
  }

  int getNumVertices () {
    return (int) vertToVariantMap->size();
  }

  void setWeight (int vertId, int weightId, int jointId, Float weight)
  {
    VariantIter var = vertToVariantMap->getFirstVariant( vertId );
    for (; var->id != -1; ++var)
    {
      SkinVertex outVert = vertBind( outTriMesh->getVertex( var->id ) );
      outVert.jointIndex[ weightId ] = jointId;
      outVert.jointWeight[ weightId ] = weight;
    }
  }
};

/*
-----------------------------------------------------------
Exports keys for an animation between given start and end
frames on the frame scroll bar
----------------------------------------------------------*/

void exportAnimClipDof (const MDagPath objPath,
                        Float startTime, Int startKey,
                        Int numKeys, Int kps,
                        Animation *outAnim,
                        DofAnimObserver *outObsrv)
{
  //Find first transform above camera
  MFnTransform transform = objPath.transform();

  //Search for custom DOF attribute plugs
  MStatus dofStatus[5];
  MPlug plugDofNear    = transform.findPlug( "DofNear",    true, &dofStatus[0] );
  MPlug plugDofStart   = transform.findPlug( "DofStart",   true, &dofStatus[1] );
  MPlug plugDofEnd     = transform.findPlug( "DofEnd",     true, &dofStatus[2] );
  MPlug plugDofFar     = transform.findPlug( "DofFar",     true, &dofStatus[3] );
  MPlug plugDofEnabled = transform.findPlug( "DofEnabled", true, &dofStatus[4] );
  if (dofStatus[0] != MStatus::kSuccess ||
      dofStatus[1] != MStatus::kSuccess ||
      dofStatus[2] != MStatus::kSuccess ||
      dofStatus[3] != MStatus::kSuccess ||
      dofStatus[4] != MStatus::kSuccess)
    return;
  
  //Create range animation tracks
  FloatAnimTrack *outTrackF[4];
  for (int t=0; t<4; ++t) {
    outTrackF[ t ] = new FloatAnimTrack;
    outAnim->addTrack( outTrackF[ t ], startKey );
    outObsrv->bindTrack( outAnim, outAnim->getNumTracks()-1, t );
  }

  //Create enabled animation track
  BoolAnimTrack *outTrackB = new BoolAnimTrack;
  outAnim->addTrack( outTrackB, startKey );
  outObsrv->bindTrack( outAnim, outAnim->getNumTracks()-1, 4 );

  //Find key step in seconds
  Float keyTime = (1.0f / kps);
  
  //Walk the animation keys
  for (Int k=0; k < numKeys; ++k)
  {
    //Set current time in seconds
    MAnimControl animCtrl;
    MTime now( startTime + k * keyTime, MTime::kSeconds );
    animCtrl.setCurrentTime( now );

    //Get values off the plugs
    bool dofEnabled = false;
    Float dofNear=0.0f, dofStart=0.0f, dofEnd=0.0f, dofFar=0.0f;
    plugDofNear.getValue( dofNear );
    plugDofStart.getValue( dofStart );
    plugDofEnd.getValue( dofEnd );
    plugDofFar.getValue( dofFar );
    plugDofEnabled.getValue( dofEnabled );

    //Add keyframes
    DofParams dofParams = exportDofParams( dofNear, dofStart, dofEnd, dofFar );
    outTrackF[ 0 ]->addKey( dofParams.focusCenter );
    outTrackF[ 1 ]->addKey( dofParams.focusRange );
    outTrackF[ 2 ]->addKey( dofParams.falloffNear );
    outTrackF[ 3 ]->addKey( dofParams.falloffFar );
    outTrackB->addKey( dofEnabled );
  }
}

/*
-----------------------------------------------------------
Exports keys for an animation between given start and end
frames on the frame scroll bar
----------------------------------------------------------*/

void exportSkinAnimClip (const MDagPath objPath,
                         Float startTime, Int startKey,
                         Int numKeys, Int kps,
                         Animation *outAnim,
                         AnimObserver *outObsrv)
{
  ArrayList<MObject> nodeTree;
  ArrayList<SkinJoint> jointTree;
  MObject meshNode;
  MObject skinNode;
  MObject jointRoot;

  //Build joint tree
  meshNode = objPath.node();
  if (!findSkinForMesh( meshNode, skinNode )) return;
  if (!findSkinJointRoot( skinNode, jointRoot )) return;
  buildJointTree( jointRoot, nodeTree, jointTree );

  //Create animation tracks for every joint
  ArrayList< QuatAnimTrack* > tracksR;
  ArrayList< Vec3AnimTrack* > tracksT;

  for (UintSize n=0; n<nodeTree.size(); ++n)
  {
    //Create rotation track
    QuatAnimTrack *trackR = new QuatAnimTrack;
    outAnim->addTrack( trackR, startKey );
    outObsrv->bindTrack( outAnim, outAnim->getNumTracks()-1, (Int) n );
    tracksR.pushBack( trackR );

    //Create translation track
    Vec3AnimTrack *trackT = new Vec3AnimTrack;
    outAnim->addTrack( trackT, startKey );
    outObsrv->bindTrack( outAnim, outAnim->getNumTracks()-1, (Int) n );
    tracksT.pushBack( trackT );
  }

  //Find key step in seconds
  Float keyTime = (1.0f / kps);
  
  //Walk the animation keys
  for (Int k=0; k < numKeys; ++k)
  {
    //Set current time in seconds
    MAnimControl animCtrl;
    MTime now( startTime + k * keyTime, MTime::kSeconds );
    animCtrl.setCurrentTime( now );

    //Walk the joint node tree
    for (UintSize n=0; n<nodeTree.size(); ++n)
    {
      //Break transformation into animateable components
      Quat keyR; Vector3 keyT; Matrix4x4 keyS;
      exportTransformJoint( nodeTree[n], keyR, keyT, keyS );

      //Add keyframes
      tracksR[ n ]->addKey( keyR );
      tracksT[ n ]->addKey( keyT );
    }
  }
}

/*
-----------------------------------------------------------
Exports keys for an animation between given start and end
frames on the frame scroll bar
----------------------------------------------------------*/

void exportAnimClip (const MDagPath objPath,
                     Float startTime, Int startKey,
                     Int numKeys, Int kps,
                     Animation *outAnim,
                     ActorAnimObserver *outObsrv)
{
  //Set fixed scale
  double scale[3];
  MTransformationMatrix mat( objPath.inclusiveMatrix() );
  mat.getScale( scale, MSpace::kTransform );
  outObsrv->valueS = exportScale( scale );

  //Add tracks to animation and bind observer to them
  QuatAnimTrack* outTrackR = new QuatAnimTrack;
  outAnim->addTrack( outTrackR, startKey );
  outObsrv->bindTrack( outAnim, outAnim->getNumTracks()-1 );

  Vec3AnimTrack* outTrackT = new Vec3AnimTrack;
  outAnim->addTrack( outTrackT, startKey );
  outObsrv->bindTrack( outAnim, outAnim->getNumTracks()-1 );

  //Find key step in seconds
  Float keyTime = (1.0f / kps);
  
  //Walk the animation keys
  for (Int k=0; k < numKeys; ++k)
  {
    //Set current time in seconds
    MAnimControl animCtrl;
    MTime now( startTime + k * keyTime, MTime::kSeconds );
    animCtrl.setCurrentTime( now );

    //Break transformation into animateable components
    Quat keyR; Vector3 keyT; Matrix4x4 keyS;
    exportTransformMatrix( objPath.inclusiveMatrix(), keyR, keyT, keyS );

    //Add keyframes
    outTrackR->addKey( keyR );
    outTrackT->addKey( keyT );
  }
}
/*
class Test : public AnimObserver
{
  DECLARE_SERIAL_SUBCLASS( Test, AnimObserver );
  DECLARE_END;

public:
  Test () {};
  Test (SM *sm) : AnimObserver(sm) {}
};

DEFINE_SERIAL_CLASS( Test, ClassID( 1, 2, 3, 4 ));
*/

Animation* exportAnimation (int kps, MayaAnimDummy *anim)
{
  //Store animation properties
  Animation* outAnim = new Animation;
  outAnim->name = anim->name;
  outAnim->duration = anim->endTime - anim->startTime;
  outAnim->kps = kps;

  //Walk the list of clips to export
  for (UintSize c=0; c<anim->clips.size(); ++c)
  {
    MayaClipDummy *clip = anim->clips[ c ];

    //Make sure the node exists in the scene
    MDagPath path;
    MSelectionList matches;
    MGlobal::getSelectionListByName( clip->nodePath.buffer(), matches);
    if (matches.getDagPath( 0, path ) != MStatus::kSuccess) {
      trace( "ExportAnimation: node '" + clip->nodePath + "' not found in the scene!" );
      continue;
    }
    
    //Make sure the actor was exported in the scene
    Actor3D *actor = findActorByName( clip->nodePath );
    if (actor == NULL) {
      trace( "ExportAnimation: actor '" + clip->nodePath + "' missing in the export list!" );
      continue;
    }

    //Find the actual node to export below the given transform

    /*
    MFn::Type type;
    if (SafeCast( SkinMeshActor, actor ) != NULL)
      type =
    if (SafeCast( TriMeshActor, actor ) != NULL)
      type = MFn::kMesh;
    else if (SafeCast( Camera, actor ) != NULL)
      type = MFn::kCamera;
    else if (SafeCast( Light, actor ) != NULL)
      type = MFn::kLight;
    */

    //Localize clip times to animation range
    Float clipLocalStartTime = Util::Max( 0.0f, clip->startTime - anim->startTime );
    Float clipLocalEndTime = Util::Max( 0.0f, clip->endTime - anim->startTime );

    //Round the start / end times to full keys based on animation kps
    Int clipLocalStartKey = (Int) FLOOR( clipLocalStartTime * kps );
    Int clipLocalEndKey = (Int) CEIL( clipLocalEndTime * kps );
    Int clipNumKeys = clipLocalEndKey - clipLocalStartKey + 1;

    //Find global key-aligned start / end times
    Float clipStartTime = anim->startTime + (Float) clipLocalStartKey / kps;
    Float clipEndTime = anim->startTime + (Float) clipLocalEndKey / kps;

    //Check if it's an actor with skin
    SkinMeshActor *skinActor = SafeCast( SkinMeshActor, actor );    
    if (skinActor != NULL)
    {
      //Create an observer for this object
      SkinAnimObserver *outObsrv = new SkinAnimObserver;
      outObsrv->actor = skinActor;
      outAnim->addObserver( outObsrv );

      //Export keys for this track
      exportSkinAnimClip( path,
        clipStartTime, clipLocalStartKey,
        clipNumKeys, kps,
        outAnim, outObsrv );
    }
    else
    {
      //Create an observer for this object
      ActorAnimObserver *outObsrv = new ActorAnimObserver;
      outObsrv->actor = actor;
      outAnim->addObserver( outObsrv );

      //Export keys for this track
      exportAnimClip( path,
        clipStartTime, clipLocalStartKey,
        clipNumKeys, kps,
        outAnim, outObsrv );
    }

    //Check if it's a camera
    Camera3D *camera = SafeCast( Camera3D, actor );
    if (camera != NULL)
    {
      //Create a DOF observer for it
      DofAnimObserver *outObsrv = new DofAnimObserver;
      outObsrv->camera = camera;
      outAnim->addObserver( outObsrv );

      //Export keys for this clip
      exportAnimClipDof( path,
        clipStartTime, clipLocalStartKey,
        clipNumKeys, kps,
        outAnim, outObsrv );
    }
  }

  //Walk the list of events to export
  for (UintSize e=0; e<anim->events.size(); ++e)
  {
    //Export event
    MayaEventDummy *evt = anim->events[ e ];
    AnimEvent *outEvt = new AnimEvent( evt->name, evt->time );
    outAnim->addEvent( outEvt );
  }

  return outAnim;
}

/*
------------------------------------------------------------
Export keys for character animation
------------------------------------------------------------*/

void exportAnimKeys (const ArrayList<MObject> &nodeTree,
                     int start, int end, int fps,
                     Animation *outAnim)
{
  //Constructs with current UI units by default
  MTime time;
  
  //Find start time in milliseconds
  time.setValue( start );
  double startMsec = time.as( MTime::kMilliseconds );

  //Find end time in milliseconds
  time.setValue( end );
  double endMsec = time.as( MTime::kMilliseconds );

  //Find iteration step in milliseconds
  double stepMsec = 1000 * (1.0 / fps);
  double stepSec = (1.0 / fps);
  double durationSec = (endMsec - startMsec) / 1000.0f;

  trace( "exportAnimKeys: frame time "
    + CharString::FFloat( (float) stepMsec ) + " milliseconds." );
  trace( "exportAnimKeys: total time "
    + CharString::FFloat( (float) durationSec) + " seconds." );

  //Create animation tracks for every bone
  outAnim->duration = (Float) durationSec;
  outAnim->kps = fps;

  ArrayList< QuatAnimTrack* > tracksR;
  ArrayList< Vec3AnimTrack* > tracksT;

  for (UintSize n=0; n<nodeTree.size(); ++n)
  {
    QuatAnimTrack *trackR = new QuatAnimTrack;
    outAnim->addTrack( trackR );
    tracksR.pushBack( trackR );

    Vec3AnimTrack *trackT = new Vec3AnimTrack;
    outAnim->addTrack( trackT );
    tracksT.pushBack( trackT );
  }

  //Walk the animation frames
  int numKeys = 0;
  for (double t=startMsec; t<=endMsec; t+=stepMsec, numKeys++)
  {
    //Set current time in milliseconds
    MAnimControl animCtrl;
    MTime now( t, MTime::kMilliseconds );
    animCtrl.setCurrentTime( now );

    //Walk the joint node tree
    for (UintSize n=0; n<nodeTree.size(); ++n)
    {
      //Add keys to tracks
      Quat keyR; Vector3 keyT; Matrix4x4 keyS;
      exportTransformJoint( nodeTree[n], keyR, keyT, keyS );

      tracksR[ n ]->addKey( keyR );
      tracksT[ n ]->addKey( keyT );
    }
  }

  trace( "exportAnimKeys: exported "
    + CharString::FInt( numKeys ) + " keys." );
}

/*
------------------------------------------------------------
Export character animation
------------------------------------------------------------*/

Animation* exportSkinAnimation (int start, int end, int fps)
{
  ArrayList<MObject> nodeTree;
  ArrayList<SkinJoint> jointTree;
  MObject meshNode;
  MObject skinNode;
  MObject jointRoot;
  
  if (!findNodeInSelection( MFn::kMesh, meshNode )) {
    setStatus( "Please select a mesh node!" );
    return NULL;
  }

  if (!findSkinForMesh( meshNode, skinNode )) {
    setStatus( "Please select a mesh with skin!" );
    return NULL;
  }

  if (!findSkinJointRoot( skinNode, jointRoot )) {
    setStatus( "Skin doesn't have any influences!" );
    return NULL;
  }

  //Build bone tree
  buildJointTree( jointRoot, nodeTree, jointTree );

  //Export animation
  Animation *outAnim = new Animation;
  exportAnimKeys( nodeTree, start, end, fps, outAnim );

  return outAnim;
}

/*
--------------------------------------------------------
Export mesh with skin
--------------------------------------------------------*/

Character* exportCharacter (const MObject &meshNode, bool tangents)
{
  ArrayList<MObject> nodeTree;
  ArrayList<SkinJoint> jointTree;
  ArrayList<Uint32> skinToTreeMap;
  MObject skinNode;
  MObject skinMesh;
  MObject jointRoot;

  if (!findSkinForMesh( meshNode, skinNode )) {
    setStatus( "Please select a mesh with skin!" );
    return NULL;
  }

  if (!findSkinJointRoot( skinNode, jointRoot )) {
    setStatus( "Skin doesn't have any influences!" );
    return NULL;
  }

  if (!findSkinPoseMesh( skinNode, skinMesh ))
    return NULL;

  //Export joint tree and pose rotations
  buildJointTree( jointRoot, nodeTree, jointTree );
  buildSkinToTreeMap( skinNode, nodeTree, skinToTreeMap );
  exportSkinPose( skinNode, jointTree, skinToTreeMap );

  //Prepare vertex format
  VertexFormat format;
  format.addMember( ShaderData::TexCoord2 );
  format.addMember( ShaderData::Normal );
  format.addMember( ShaderData::Coord3 );
  format.addMember( ShaderData::JointIndex );
  format.addMember( ShaderData::JointWeight );
  if (tangents)
  {
    format.addMember( ShaderData::Tangent );
    format.addMember( ShaderData::Bitangent );
  }

  //Export pose mesh
  VertToVariantMap vertToVariantMap;
  
  SkinTriMesh *outTriMesh = new SkinTriMesh;
  outTriMesh->setFormat( format );

  TriMeshExporter meshExporter( outTriMesh, vertToVariantMap );
  meshExporter.exportMesh( skinMesh );

  //Export skin weights
  TriWeightExporter weightExporter( outTriMesh, vertToVariantMap );
  weightExporter.exportSkinWeights( skinNode, meshNode, skinToTreeMap );

  //Construct character
  Character *character = new Character;
  character->pose = new SkinPose;
  character->pose->joints.pushListBack( &jointTree );

  //Split into 24-bone sub meshes
  trace( "exportCharacter: Splitting by bone limit..." );

  SkinSuperToSubMesh splitter( outTriMesh );
  splitter.splitByBoneLimit( 24 );

  UintSize numMeshes = splitter.getSubMeshCount();
  for (UintSize m=0; m<numMeshes; ++m)
  {
    SkinTriMesh *mesh = (SkinTriMesh*) splitter.getSubMesh(m);
    mesh->updateBoundingBox();
    character->meshes.pushBack( mesh );
  }

  trace( "exportCharacter: Generated " + CharString::FInt( (int)numMeshes ) + " sub-meshes." );

  return character;
}

/*
--------------------------------------------------------
Export static mesh
--------------------------------------------------------*/

TriMesh* exportMesh (const MObject &meshNode, bool tangents)
{
  //Prepare vertex format
  VertexFormat format;
  format.addMember( ShaderData::TexCoord2 );
  format.addMember( ShaderData::Normal );
  format.addMember( ShaderData::Coord3 );
  if (tangents)
  {
    format.addMember( ShaderData::Tangent );
    format.addMember( ShaderData::Bitangent );
  }

  //Export mesh data
  VertToVariantMap vertToVariantMap;

  TriMesh *outTriMesh = new TriMesh;
  outTriMesh->setFormat( format );

  TriMeshExporter meshExporter( outTriMesh, vertToVariantMap );
  meshExporter.exportMesh( meshNode );

  outTriMesh->updateBoundingBox();
  return outTriMesh;
}
