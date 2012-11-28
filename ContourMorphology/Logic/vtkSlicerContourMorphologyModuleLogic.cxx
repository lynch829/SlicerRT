/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kevin Wang, Radiation Medicine Program, 
  University Health Network and was supported by Cancer Care Ontario (CCO)'s ACRU program 
  with funds provided by the Ontario Ministry of Health and Long-Term Care
  and Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).

==============================================================================*/

// ContourMorpholgyModule Logic includes
#include "vtkSlicerContourMorphologyModuleLogic.h"
#include "vtkMRMLContourMorphologyNode.h"

// SlicerRT includes
#include "SlicerRtCommon.h"

// MRML includes
#include <vtkMRMLContourNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLVolumeDisplayNode.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkImageData.h>
#include <vtkImageMarchingCubes.h>
#include <vtkImageChangeInformation.h>
#include <vtkSmartPointer.h>
#include <vtkLookupTable.h>
#include <vtkTriangleFilter.h>
#include <vtkDecimatePro.h>
#include <vtkPolyDataNormals.h>
#include <vtkLookupTable.h>
#include <vtkColorTransferFunction.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkImageContinuousDilate3D.h>
#include <vtkImageContinuousErode3D.h>
#include <vtkImageLogic.h>
#include <vtkImageAccumulate.h>

// STD includes
#include <cassert>

#define THRESHOLD 0.001

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerContourMorphologyModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerContourMorphologyModuleLogic::vtkSlicerContourMorphologyModuleLogic()
{
  this->ContourMorphologyNode = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerContourMorphologyModuleLogic::~vtkSlicerContourMorphologyModuleLogic()
{
  vtkSetAndObserveMRMLNodeMacro(this->ContourMorphologyNode, NULL);
}

//----------------------------------------------------------------------------
void vtkSlicerContourMorphologyModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkSlicerContourMorphologyModuleLogic::SetAndObserveContourMorphologyNode(vtkMRMLContourMorphologyNode *node)
{
  vtkSetAndObserveMRMLNodeMacro(this->ContourMorphologyNode, node);
}

//---------------------------------------------------------------------------
void vtkSlicerContourMorphologyModuleLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  events->InsertNextValue(vtkMRMLScene::EndCloseEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerContourMorphologyModuleLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    return;
  }
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLContourMorphologyNode>::New());
}

//---------------------------------------------------------------------------
void vtkSlicerContourMorphologyModuleLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkSlicerContourMorphologyModuleLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene() || !this->ContourMorphologyNode)
    {
    return;
    }

  // if the scene is still updating, jump out
  if (this->GetMRMLScene()->IsBatchProcessing())
  {
    return;
  }

  if (node->IsA("vtkMRMLContourNode") || node->IsA("vtkMRMLContourMorphologyNode"))
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerContourMorphologyModuleLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene() || !this->ContourMorphologyNode)
    {
    return;
    }

  // if the scene is still updating, jump out
  if (this->GetMRMLScene()->IsBatchProcessing())
  {
    return;
  }

  if (node->IsA("vtkMRMLContourNode") || node->IsA("vtkMRMLContourMorphologyNode"))
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerContourMorphologyModuleLogic::OnMRMLSceneEndImport()
{
  // If we have a parameter node select it
  vtkMRMLContourMorphologyNode *paramNode = NULL;
  vtkMRMLNode *node = this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLContourMorphologyNode");
  if (node)
    {
    paramNode = vtkMRMLContourMorphologyNode::SafeDownCast(node);
    vtkSetAndObserveMRMLNodeMacro(this->ContourMorphologyNode, paramNode);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerContourMorphologyModuleLogic::OnMRMLSceneEndClose()
{
  this->Modified();
}

//---------------------------------------------------------------------------
bool vtkSlicerContourMorphologyModuleLogic::ContourContainsLabelmap()
{
  if (!this->GetMRMLScene() || !this->ContourMorphologyNode)
  {
    return false;
  }

  vtkMRMLContourNode* contourNode = vtkMRMLContourNode::SafeDownCast(
    this->GetMRMLScene()->GetNodeByID(this->ContourMorphologyNode->GetContourNodeID()));

  if (contourNode->GetActiveRepresentationType() == vtkMRMLContourNode::IndexedLabelmap)
  {
    return true;
  }

  return false;
}

//---------------------------------------------------------------------------
int vtkSlicerContourMorphologyModuleLogic::MorphContour()
{
  double originX, originY, originZ;
  double spacingX, spacingY, spacingZ;
  int dimensions[3] = {0, 0, 0};
  double originX2, originY2, originZ2;
  double spacingX2, spacingY2, spacingZ2;
  int dimensions2[3] = {0, 0, 0};

  // Make sure inputs are initialized
  if (!this->GetMRMLScene())
  {
    return 0;
  }

  vtkMRMLContourNode* contourNode = vtkMRMLContourNode::SafeDownCast(
    this->GetMRMLScene()->GetNodeByID(this->ContourMorphologyNode->GetContourNodeID()));
  if (contourNode->GetActiveRepresentationType() != vtkMRMLContourNode::IndexedLabelmap)
  {
    vtkErrorMacro("Cannot Morph the contour when its representation is not labelmap!");
    return 0;
  }
  vtkMRMLScalarVolumeNode* volumeNode = contourNode->GetIndexedLabelmapVolumeNode();
  volumeNode->GetOrigin(originX, originY, originZ); 
  volumeNode->GetSpacing(spacingX, spacingY, spacingZ);
  volumeNode->GetImageData()->GetDimensions(dimensions);

  vtkSmartPointer<vtkImageAccumulate> histogram = vtkSmartPointer<vtkImageAccumulate>::New();
  histogram->SetInput(volumeNode->GetImageData());
  histogram->Update();
  double valueMax = histogram->GetMax()[0];

  vtkMRMLContourNode* outputContourNode = vtkMRMLContourNode::SafeDownCast(
    this->GetMRMLScene()->GetNodeByID(this->ContourMorphologyNode->GetOutputContourNodeID()));

  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState); 

  vtkMRMLScalarVolumeNode* volumeNode2;
  int op = this->ContourMorphologyNode->GetOperation();
  if(op == SLICERRT_UNION || op == SLICERRT_INTERSECT || op == SLICERRT_SUBTRACT)
  {
    vtkMRMLContourNode* secondaryContourNode = vtkMRMLContourNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(this->ContourMorphologyNode->GetSecondaryContourNodeID()));
    if (contourNode->GetActiveRepresentationType() != vtkMRMLContourNode::IndexedLabelmap)
    {
      vtkErrorMacro("Cannot Morph the contour when its representation is not labelmap!");
      return 0;
    }
    volumeNode2 = secondaryContourNode->GetIndexedLabelmapVolumeNode();
    volumeNode2->GetOrigin(originX2, originY2, originZ2); 
    volumeNode2->GetSpacing(spacingX2, spacingY2, spacingZ2);
    volumeNode2->GetImageData()->GetDimensions(dimensions2);
    // Make sure inputs are initialized
    if (abs(originX-originX2) > THRESHOLD || abs(originY-originY2) > THRESHOLD || abs(originZ-originZ2) > THRESHOLD || 
        abs(spacingX-spacingX2) > THRESHOLD || abs(spacingY-spacingY2) > THRESHOLD || abs(spacingZ-spacingZ2) > THRESHOLD ||
        abs(dimensions[0]-dimensions2[0]) > THRESHOLD || abs(dimensions[1]-dimensions2[1]) > THRESHOLD || abs(dimensions[2]-dimensions2[2]) >THRESHOLD)
    {
      vtkErrorMacro("Image geometry information do not match");
      return 0;
    }
  }

  // to do ....
  vtkSmartPointer<vtkMRMLScalarVolumeNode> outputIndexedLabelmapVolumeNode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
  outputIndexedLabelmapVolumeNode->CopyOrientation( volumeNode );

  vtkSmartPointer<vtkImageData> tempImageData = NULL;
  vtkSmartPointer<vtkImageContinuousDilate3D> dilateFilter = vtkSmartPointer<vtkImageContinuousDilate3D>::New();
  vtkSmartPointer<vtkImageContinuousErode3D> erodeFilter = vtkSmartPointer<vtkImageContinuousErode3D>::New();
  vtkSmartPointer<vtkImageLogic> logicFilter = vtkSmartPointer<vtkImageLogic>::New();
  vtkSmartPointer<vtkImageLogic> logicFilter2 = vtkSmartPointer<vtkImageLogic>::New();

  // temp work around for calculating the kernel size
  
  double spacing[3] = {0,0,0};
  volumeNode->GetSpacing(spacing);
  int kernelSize[3] = {1,1,1};
  kernelSize[0] = (int)(this->GetContourMorphologyNode()->GetXSize()/spacing[0]);
  kernelSize[1] = (int)(this->GetContourMorphologyNode()->GetYSize()/spacing[1]);
  kernelSize[2] = (int)(this->GetContourMorphologyNode()->GetZSize()/spacing[2]);
  switch (op) 
  {
    case SLICERRT_EXPAND:
      dilateFilter->SetInput(volumeNode->GetImageData());
      dilateFilter->SetKernelSize(kernelSize[0], kernelSize[1], kernelSize[2]);
      dilateFilter->Update();
      tempImageData = dilateFilter->GetOutput();
      break;
    case SLICERRT_SHRINK:
      erodeFilter->SetInput(volumeNode->GetImageData());
      erodeFilter->SetKernelSize(kernelSize[0], kernelSize[1], kernelSize[2]);
      erodeFilter->Update();
      tempImageData = erodeFilter->GetOutput();
      break;
    case SLICERRT_UNION:
      logicFilter->SetInput1(volumeNode->GetImageData());
      logicFilter->SetInput2(volumeNode2->GetImageData());
      logicFilter->SetOperationToOr();
      logicFilter->SetOutputTrueValue(valueMax);
      logicFilter->Update();
      tempImageData = logicFilter->GetOutput();
      break;
    case SLICERRT_INTERSECT:
      logicFilter->SetInput1(volumeNode->GetImageData());
      logicFilter->SetInput2(volumeNode2->GetImageData());
      logicFilter->SetOperationToAnd();
      logicFilter->SetOutputTrueValue(valueMax);
      logicFilter->Update();
      tempImageData = logicFilter->GetOutput();
      break;
    case SLICERRT_SUBTRACT:
      logicFilter->SetInput1(volumeNode2->GetImageData());
      logicFilter->SetOperationToNot();
      logicFilter->SetOutputTrueValue(valueMax);
      logicFilter->Update();

      logicFilter2->SetInput1(volumeNode->GetImageData());
      logicFilter2->SetInput2(logicFilter->GetOutput());
      logicFilter2->SetOperationToAnd();
      logicFilter2->SetOutputTrueValue(valueMax);
      tempImageData = logicFilter2->GetOutput();
      break;
  }

  std::string outputIndexedLabelmapVolumeNodeName = std::string(outputContourNode->GetName()) + std::string(" - Labelmap");
  outputIndexedLabelmapVolumeNodeName = this->GetMRMLScene()->GenerateUniqueName(outputIndexedLabelmapVolumeNodeName);
  this->GetMRMLScene()->AddNode(outputIndexedLabelmapVolumeNode);

  outputIndexedLabelmapVolumeNode->SetAndObserveTransformNodeID( outputIndexedLabelmapVolumeNode->GetTransformNodeID() );
  outputIndexedLabelmapVolumeNode->SetName( outputIndexedLabelmapVolumeNodeName.c_str() );
  outputIndexedLabelmapVolumeNode->SetAndObserveImageData( tempImageData );
  outputIndexedLabelmapVolumeNode->LabelMapOn();
  outputIndexedLabelmapVolumeNode->HideFromEditorsOff();

  // Create display node
  vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode> labelmapDisplayNode = vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode>::New();
  labelmapDisplayNode = vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(this->GetMRMLScene()->AddNode(labelmapDisplayNode));
  outputIndexedLabelmapVolumeNodeName.append("Display");
  labelmapDisplayNode->SetName(outputIndexedLabelmapVolumeNodeName.c_str());

  if (this->GetMRMLScene()->GetNodeByID("vtkMRMLColorTableNodeLabels") != NULL)
  {
    labelmapDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeLabels");
  }
  labelmapDisplayNode->SetVisibility(1);
  
  outputIndexedLabelmapVolumeNode->SetAndObserveDisplayNodeID( labelmapDisplayNode->GetID() );

  outputContourNode->SetAndObserveIndexedLabelmapVolumeNodeId(outputIndexedLabelmapVolumeNode->GetID());
  outputContourNode->SetActiveRepresentationByNode(outputIndexedLabelmapVolumeNode);

  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState); 

  return 0;
}