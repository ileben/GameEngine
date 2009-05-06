global proc GCmdOpen()
{
  if (`window -exists GWndMain`) return;
  if (`windowPref -exists GWndMain`)
  { windowPref -remove GWndMain; };
  
  trace "Opening GExporter...";
  window -title "GameExporter" -widthHeight 199 700 GWndMain;
  
  scrollLayout -childResizable true -width 200;
  columnLayout -adjustableColumn true -cw 100 -cat "both" 0 -rs 0 GColMain;
  
  frameLayout -label "Mesh" -bv true -bs "etchedIn" -mh 10 -mw 10 -collapsable true -vis false GFrmMesh;
  columnLayout -columnAlign "left" -adjustableColumn true -rs 2; 
    text -label "Mesh node:"; 
    button -label "<Pick selected>" -align "center" -command "GCmdPickMesh" GBtnPickMesh; 
    separator -height 10 -style "none"; 
    
  setParent GColMain; 
  frameLayout -label "Skin" -bv true -bs "etchedIn" -mh 10 -mw 10 -collapsable true -vis false GFrmSkin; 
  columnLayout -columnAlign "left" -adjustableColumn true -rs 2; 
    text -label "Root joint:"; 
    button -label "<Pick selected>" -align "center" -command "GCmdPickJoint" GBtnPickJoint; 
    
  setParent GColMain; 
  frameLayout -label "Static Mesh \\ Skin Pose" -bv true -bs "etchedIn" -mh 10 -mw 10 -collapsable true; 
  columnLayout -columnAlign "left" -adjustableColumn true -rs 2 GColPose; 
    checkBox -label "Export tangents" -onCommand "GCmdTangentsOn" -offCommand "GCmdTangentsOff" GChkTangents;
    checkBox -label "Export with skin" -onCommand "GCmdSkinOn" -offCommand "GCmdSkinOff" GChkSkin; 
    separator -height 10 -style "none"; 
    text -label "File name:"; 
    
    rowLayout -nc 2 -adj 1 -cw 2 20 -cat 2 "both" 0; 
    textField GTxtFile; 
    button -label "..." -align "center" -command "GCmdBrowseFile"; 
    
    setParent GColPose; 
    separator -height 10 -style "none"; 
    button -label "Export" -align "center" -command "GCmdExport"; 
    
  setParent GColMain; 
  frameLayout -label "New Animation" -bv true -bs "etchedIn" -mh 10 -mw 10 -collapsable true; 
  columnLayout -columnAlign "left" -adjustableColumn true -rs 2 GColNewAnim; 
    text -label "Name:"; 
    textField GTxtAnimName; 
    
    rowLayout -nc 2 -adj 2 -cw 1 70 -cat 1 "both" 0; 
    text -label "Start frame:"; 
    textField GTxtAnimStart; 
    
    setParent GColNewAnim; 
    rowLayout -nc 2 -adj 2 -cw 1 70 -cat 1 "both" 0; 
    text -label "End frame:"; 
    textField GTxtAnimEnd; 
    
    setParent GColNewAnim; 
    separator -height 10 -style "none"; 
    rowLayout -nc 2 -adj 2 -cw 1 70 -ct2 "both" "both" -cl2 "center" "center"; 
    button -label "Clear" -align "center" -command "GCmdClearAnim" -enable false GBtnClearAnim; 
    button -label "Process" -align "center" -command "GCmdProcessAnim" GBtnProcessAnim;
    
  setParent GColMain; 
  frameLayout -label "Character" -bv true -bs "etchedIn" -mh 10 -mw 10 -collapsable true;
  columnLayout -columnAlign "left" -adjustableColumn true -rs 2 GColAnim; 
    text -label "File name:"; 
    
    rowLayout -nc 2 -adj 1 -cw 2 20 -cat 2 "both" 0; 
    textField -enable false GTxtSkinFile; 
    button -label "..." -align "center" -command "GCmdBrowseSkinFile"; 
    
    setParent GColAnim; 
    text -label "Animations:"; 
    textScrollList -height 80 GLstAnim; 
    
    rowLayout -nc 2 -adj 2 -cw 1 70 -ct2 "both" "both" -cl2 "center" "center"; 
    button -label "Remove" -align "center" -command "GCmdRemoveAnim"; 
    button -label "Add New" -align "center" -command "GCmdAddAnim"; 
    
    setParent GColAnim; 
    separator -height 10 -style "none"; 
    button -label "Save" -align "center" -command "GCmdSaveChar"; 
    
  setParent GColMain; 
  frameLayout -label "Status" -bv true -bs "etchedIn" -mh 10 -mw 10 -collapsable true; 
    text -label "<Awaiting Command>" GTxtStatus; 
    
  showWindow GWndMain; 
  window -edit -width 200 GWndMain; 
  GCmdRestoreGui; 
} 
global proc GCmdClose() 
{ 
  trace "Closing GExporter..."; 
  if (`window -exists GWndMain`) 
  { deleteUI GWndMain; }; 
  if (`windowPref -exists GWndMain`) 
  { windowPref -remove GWndMain; }; 
} 
global proc GCmdCreateMenu() 
{ 
  trace "Creating GExporter menu..."; 
  global string $gMainWindow; 
	menu -parent $gMainWindow -label "GExporter" GMenuMain; 
	menuItem -label "Open" -command "GCmdOpen" GMenuOpen; 
} 
global proc GCmdDestroyMenu() 
{ 
  if (`menu -exists GMenuMain`) 
  { deleteUI GMenuMain; };
}
