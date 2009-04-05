#include "geMaya.h"

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

Quat exportQuat (const MQuaternion &q)
{
  return Quat( (Float) q.x, (Float) q.y, (Float) -q.z, (Float) -q.w );
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
}

void exportTransformJoint (const MObject &o, Quat &R, Vector3 &T, Matrix4x4 &S)
{
  MFnIkJoint joint( o );
  MQuaternion jointR, jointO;
  joint.getRotation( jointR );
  joint.getOrientation( jointO );
  MVector jointT = joint.getTranslation( MSpace::kTransform );
  R = exportQuat( jointR * jointO );
  T = exportVector( jointT );
  S.setIdentity();
}

/*
-----------------------------------------------
Exports the mesh geometry and UV data.
-----------------------------------------------*/

void exportMesh (const MObject &meshNode,
                 SPolyMesh *outPolyMesh,
                 TexMesh *outTexMesh)
{
  MStatus status;
  MFloatPointArray meshPoints;
  MFloatArray meshPointsU;
  MFloatArray meshPointsV;
  MIntArray meshNumCorners;
  MIntArray meshIndices;
  MIntArray meshFaceIndices;
  MIntArray meshFaceUVIndices;
  Uint meshNumPoints;
  Uint meshNumPointsUV;
  Uint meshNumFaces;

  ArrayList<SPolyMesh::Vertex*> outVerts;
  ArrayList<SPolyMesh::Vertex*> outFaceVerts;
  ArrayList<TexMesh::Vertex*> outTexVerts;
  ArrayList<TexMesh::Vertex*> outTexFaceVerts;

  int invalidIndices = 0;
  int invalidTexIndices = 0;
  int invalidFaces = 0;
  int invalidTexFaces = 0;

  //Get mesh points (in world space if possible)
  MFnMesh mesh( meshNode, &status );
  status = mesh.getPoints( meshPoints, MSpace::kWorld );
  if (status == MStatus::kInvalidParameter) {
    status = mesh.getPoints( meshPoints, MSpace::kPostTransform );
    if (status == MStatus::kInvalidParameter) {
      status = mesh.getPoints( meshPoints, MSpace::kObject );
    }}

  //Get mesh UV points
  mesh.getUVs( meshPointsU, meshPointsV );
  meshNumPoints = mesh.numVertices();
  meshNumPointsUV = mesh.numUVs();
  meshNumFaces = mesh.numPolygons();

  //Walk the mesh points
  for (Uint p=0; p<meshNumPoints; ++p)
  {
    //Add new vertex to the mesh
    SPolyMesh::Vertex *outVert = outPolyMesh->addVertex();
    outVert->point = exportPoint( meshPoints[ p ] );
    outVerts.pushBack( outVert );
  }

  //Walk the mesh UV points
  for (Uint uv=0; uv<meshNumPointsUV; ++uv)
  {
    TexMesh::Vertex *outTexVert = outTexMesh->addVertex();
    outTexVert->point.set( meshPointsU[ uv ], meshPointsV[ uv ] );
    outTexVerts.pushBack( outTexVert );
  }

  //Walk the mesh polygons
  for (Uint f=0; f<meshNumFaces; ++f)
  {
    //Get polygon corner indices
    meshFaceIndices.clear();
    mesh.getPolygonVertices( f, meshFaceIndices);
    Uint meshNumFaceIndices = meshFaceIndices.length();

    //Init the face vertex array
    outFaceVerts.reserve( meshNumFaceIndices );
    outFaceVerts.clear();

    //Init the UV face vertex array
    outTexFaceVerts.reserve( meshNumFaceIndices );
    outTexFaceVerts.clear();

    //Walk the polygon vertices
    for (Uint c=0; c<meshNumFaceIndices; ++c)
    {
      //Make sure we have the point available
      int index = meshFaceIndices[ c ];
      if ((UintSize)index >= outVerts.size()) {
        invalidIndices++;
        continue; }

      //Add vertex to the array
      outFaceVerts.pushBack( outVerts[ index ] );

      //Check if a UV index exists for this vertex
      int indexUV = -1; status = mesh.getPolygonUVid( f, c, indexUV );
      if (status == MStatus::kSuccess)
      {
        //Make sure we have the UV point available
        if ((UintSize)indexUV >= outTexVerts.size()) {
          invalidTexIndices++;
          continue; }

        //Add UV vertex to the array
        outTexFaceVerts.pushBack( outTexVerts[ indexUV ] );
      }
    }
    
    //Add new face to the mesh
    SPolyMesh::Face *face = outPolyMesh->addFace(
      outFaceVerts.buffer(), (int)outFaceVerts.size() );
    if (face == NULL) {
      invalidFaces++;
      continue; }
    
    //Assign face properties
    face->smoothGroups = 1;
    
    //Check if a full UV face exists for this polygon
    if (outTexFaceVerts.size() == outFaceVerts.size())
    {
      //Add new UV face to the mesh
      TexMesh::Face *texFace = outTexMesh->addFace(
        outTexFaceVerts.buffer(), (int)outTexFaceVerts.size() );
      if (texFace == NULL) {
        invalidTexFaces++;
        continue; }
    }
  }

  //Report

  if (invalidIndices > 0) {
    trace( "exportMesh: " + CharString::FInt( invalidIndices )
           + " invalid vertex indices encountered!" ); }
  if (invalidTexIndices > 0 ) {
    trace( "exportMesh: " + CharString::FInt( invalidTexIndices )
           + " invalid UV vertex indices encountered!" ); }
  if (invalidFaces > 0 ) {
    trace( "exportMesh: " + CharString::FInt( invalidFaces )
           + " invalid faces encountered!" ); }
  if (invalidTexFaces > 0) {
    trace( "exportMesh: " + CharString::FInt( invalidTexFaces )
           + " invalid UV faces encountered!" ); }

  trace( "exportMesh: exported "
         + CharString::FInt( outPolyMesh->vertexCount() ) + " vertices." );
  trace( "exportMesh: exported "
         + CharString::FInt( outPolyMesh->faceCount() ) + " faces." );
  trace( "exportMesh: exported "
         + CharString::FInt( outTexMesh->vertexCount() ) + " UV vertices." );
  trace( "exportMesh: exported "
         + CharString::FInt( outTexMesh->faceCount() ) + " UV faces." );
}

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
    trace( "findSkinForMesh: 'inMesh' plug missing or not connected!" );
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
  trace( "findSkinForMesh: no skin found on the 'inMesh' plug!" );
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
Builds the joint/bone hierarchy tree.
-----------------------------------------------------------------*/

void buildBoneTree (const MObject &jointNode,
                    ArrayList<MObject> &jointTree,
                    ArrayList<SkinBone> &boneTree)
{
  //Push root joint onto the queue
  LinkedList< MObject > dagQueue;
  dagQueue.pushBack( jointNode );
  while (!dagQueue.empty())
  {
    //Pop a node from the queue
    MObject dagObj = dagQueue.first();
    dagQueue.popFront();

    //Add the node and a bone to the tree
    jointTree.pushBack( dagObj );
    boneTree.pushBack( SkinBone() );

    //Init bone children
    SkinBone *bone = &boneTree.last();
    bone->numChildren = 0;

    //Walk node children
    MFnDagNode dagNode( dagObj );
    for (Uint c=0; c<dagNode.childCount(); ++c)
    {
      //Check if it's a kJoint type
      MObject child = dagNode.child( c );
      if (child.hasFn( MFn::kJoint ))
      {
        //Push onto the queue and add to bone
        dagQueue.pushBack( child );
        bone->numChildren++;
      }
    }
  }

  //Report
  trace(
    "buildBoneTree: exported " +
    CharString::FInt( (int) boneTree.size() ) + " bones." );
}

/*
----------------------------------------------------------
Builds an array mapping the skin influence indices to the
indices of their respective bone nodes in the joint tree.
----------------------------------------------------------*/

void buildSkinToTreeMap (const MObject &skinNode,
                         const ArrayList<MObject> &jointTree,
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
    int treeIndex = jointTree.indexOf( skinInfluencePaths[i].node() );
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
the data is stored into it for every bone influence added to the
skin at the time of binding.

The bindPreMatrix array plug elements are never removed even if
the bone influences are removed from the skin. This means the
index of a bone influence in the current influence array might
not be the same as the index of its matrix in the bindPreMatrix
plug in case any influences were removed in the middle of the
array. The index that relates the two is called "logical index"
and those indices just keep increasing when new bones are added.
------------------------------------------------------------------*/

void exportBonePose (const MObject &skinNode,
                     const ArrayList<SkinBone> &boneTree,
                     const ArrayList<Uint> &skinToTreeMap)
{
  MStatus status;
  MMatrix *invMatrices = new MMatrix[ boneTree.size() ];

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

    //Assign the inverse pose matrix to the bone
    SkinBone *bone = &boneTree[ skinToTreeMap[i] ];
    bone->worldInv = exportMatrix( invMatrix );
    invMatrices[ skinToTreeMap[i] ] = invMatrix;
  }

  //Export root pose
  SkinBone *root = &boneTree[ 0 ];
  MMatrix rootLocalMatrix = invMatrices[ 0 ].inverse();
  exportTransformMatrix( rootLocalMatrix,
    root->localR, root->localT, root->localS );

  //Walk the bone tree
  for (UintSize p=0, c=1; p<boneTree.size(); ++p)
  {
    //Walk all the children of this bone
    for (Uint32 pc=0; pc<boneTree[p].numChildren; ++pc)
    {
      //Get the local transformation matrix
      MMatrix worldMatrix = invMatrices[ c ].inverse();
      MMatrix localMatrix = worldMatrix * invMatrices[ p ];

      //Export child pose
      SkinBone *child = &boneTree[ c ];
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

void exportSkinWeights (const MObject &skinNode,
                        const MObject &meshNode,
                        SPolyMesh *outPolyMesh,
                        const ArrayList<Uint> &skinToTreeMap)
{
  ArrayList<SPolyMesh::Vertex*> outVerts;

  //Store the vertices into an array
  //(tag will hold the number of weights assigned)
  for (SPolyMesh::VertIter v( outPolyMesh ); !v.end(); ++v) {
    v->tag.id = 0;
    for (int i=0; i<4; ++i) {
      v->boneIndex[ i ] = 0;
      v->boneWeight[ i ] = 0.0f; }
    outVerts.pushBack( *v );
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
        int vertexID = component.element( c );
        if ((UintSize) vertexID > outVerts.size()) continue;
        SPolyMesh::Vertex *outVert = outVerts[ vertexID ];

        //Check if number of weights reached 4
        int numWeights = outVerts[ vertexID ]->tag.id;
        if (numWeights >= 4) {
          outVerts[ vertexID ]->tag.id++;
          continue; }

        //Add weight to the vertex
        outVert->boneIndex[ numWeights ] = skinToTreeMap[ i ];
        outVert->boneWeight[ numWeights ] = (Float) weights[ c ];
        outVert->tag.id++;
      }
    }
  }

  //Re-check vertices for errors
  Int zero = 0;
  Int many = 0;
  for (SPolyMesh::VertIter v( outPolyMesh ); !v.end(); ++v) {
    if (v->tag.id == 0) zero++;
    if (v->tag.id > 4) many++;
  }

  //Report
  if (zero > 0)
    trace( "exportSkinWeights: " + CharString::FInt( zero )
    + " vertices have no weight!" );

  if (many > 0)
    trace( "exportSkinWeights: " + CharString::FInt( many )
    + " vertices have too many weights!" );
}


void exportAnimKeys (const ArrayList<MObject> &jointTree,
                     int start, int end, int fps,
                     SkinAnim *outAnim)
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
  outAnim->duration = (float) durationSec;
  for (UintSize j=0; j<jointTree.size(); ++j)
  {
    QuatAnimTrack *trackR = new QuatAnimTrack;
    trackR->totalTime = (float) durationSec;
    trackR->frameTime = (float) stepSec;
    outAnim->tracksR.pushBack( trackR );

    Vec3AnimTrack *trackT = new Vec3AnimTrack;
    trackT->totalTime = (float) durationSec;
    trackT->frameTime = (float) stepSec;
    outAnim->tracksT.pushBack( trackT );
  }

  //Walk the animation frames
  int numKeys = 0;
  for (double t=startMsec; t<=endMsec; t+=stepMsec, numKeys++)
  {
    //Set current time in milliseconds
    MAnimControl animCtrl;
    MTime now( t, MTime::kMilliseconds );
    animCtrl.setCurrentTime( now );

    //Walk the joint tree
    for (UintSize j=0; j<jointTree.size(); ++j)
    {
      //Add keys to tracks
      Matrix4x4 S;
      QuatTrackTraits::Key keyR;
      Vec3TrackTraits::Key keyT;
      exportTransformJoint( jointTree[j], keyR.value, keyT.value, S );
      outAnim->tracksR[ j ]->keys.pushBack( keyR );
      outAnim->tracksT[ j ]->keys.pushBack( keyT );
    }
  }

  trace( "exportAnimKeys: exported "
    + CharString::FInt( numKeys ) + " keys." );
}

/*
--------------------------------------------------------
Export mesh with skin
--------------------------------------------------------*/

void exportWithSkin (void **outData, UintSize *outSize)
{
  ArrayList<MObject> jointTree;
  ArrayList<SkinBone> boneTree;
  ArrayList<Uint32> skinToTreeMap;
  MObject meshNode;
  MObject skinNode;
  MObject skinMesh;
  MObject jointRoot;
  
  if (!findNodeInSelection( MFn::kMesh, meshNode )) {
    setStatus( "Please select a mesh node!" );
    return;
  }

  if (!findSkinForMesh( meshNode, skinNode )) {
    setStatus( "Please select a mesh with skin!" );
    return;
  }

  if (!findSkinJointRoot( skinNode, jointRoot )) {
    setStatus( "Skin doesn't have any influences!" );
    return;
  }

  if (!findSkinPoseMesh( skinNode, skinMesh ))
    return;

  //Export pose bone tree
  buildBoneTree( jointRoot, jointTree, boneTree );
  buildSkinToTreeMap( skinNode, jointTree, skinToTreeMap );
  exportBonePose( skinNode, boneTree, skinToTreeMap );
  
  //Export pose mesh
  SPolyMesh *outPolyMesh = new SPolyMesh;
  TexMesh *outTexMesh = new TexMesh;
  exportMesh( skinMesh, outPolyMesh, outTexMesh );

  //Export skin weights
  exportSkinWeights( skinNode, meshNode, outPolyMesh, skinToTreeMap );

  //Triangulate
  outPolyMesh->triangulate();
  outPolyMesh->updateNormals( ShadingModel::Smooth );
  SkinTriMesh *outTriMesh = new SkinTriMesh;
  outTriMesh->fromPoly( outPolyMesh, NULL );

  //Construct character
  SkinPose *outPose = new SkinPose;
  outPose->bones.pushListBack( &boneTree );

  MaxCharacter *character = new MaxCharacter;
  character->pose = outPose;
  character->mesh = outTriMesh;

  //Serialize
  SerializeManager sm;
  sm.save( character, outData, outSize );
 
  //Cleanup
  delete outPolyMesh;
  delete outTexMesh;
  delete character;
}

/*
--------------------------------------------------------
Export static mesh
--------------------------------------------------------*/

void exportNoSkin (void **outData, UintSize *outSize)
{
  MObject meshNode;
  if (!findNodeInSelection( MFn::kMesh, meshNode )) {
    setStatus( "Please select a mesh node!" );
    return;
  }

  //Export mesh data
  SPolyMesh *outPolyMesh = new SPolyMesh;
  TexMesh *outTexMesh = new TexMesh;
  exportMesh( meshNode, outPolyMesh, outTexMesh );

  //Triangulate
  outPolyMesh->triangulate();
  outPolyMesh->updateNormals( ShadingModel::Smooth );
  TriMesh *outTriMesh = new TriMesh;
  outTriMesh->fromPoly( outPolyMesh, NULL );

  //Serialize
  SerializeManager sm;
  sm.save( outTriMesh, outData, outSize );

  //Cleanup
  delete outPolyMesh;
  delete outTexMesh;
  delete outTriMesh;
}

SkinAnim* exportAnimation (int start, int end, int fps)
{
  ArrayList<MObject> jointTree;
  ArrayList<SkinBone> boneTree;
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
  buildBoneTree( jointRoot, jointTree, boneTree );

  //Export animation
  SkinAnim *outAnim = new SkinAnim;
  exportAnimKeys( jointTree, start, end, fps, outAnim );

  return outAnim;
}
