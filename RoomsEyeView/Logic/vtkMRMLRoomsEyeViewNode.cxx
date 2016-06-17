/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// MRMLDoseAccumulation includes
#include "vtkMRMLRoomsEyeViewNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
static const char* GANTRY_TO_FIXEDREFERENCE_TRANSFORM_NODE_REFERENCE_ROLE = "gantryToFixedReferenceTransformRef";
static const char* COLLIMATOR_TO_GANTRY_TRANSFORM_NODE_REFERENCE_ROLE = "collimatorToGantryTransformRef";

static const char* LEFTIMAGINGPANEL_TO_LEFTIMAGINGPANELORIGIN_TRANSFORM_NODE_REFERENCE_ROLE = "leftImagingPanelToLeftImagingPanelTransformRef";
static const char* LEFTIMAGINGPANELORIGIN_TO_LEFTIMAGINGPANELROTATED_TRANSFORM_NODE_REFERENCE_ROLE = "leftImagingPanelOriginToLeftImagingPanelRotated";
static const char* LEFTIMAGINGPANELROTATED_TO_GANTRY_TRANSFORM_NODE_REFERENCE_ROLE = "leftImagingPanelRotatedToGantryTransformRef";
static const char* LEFTIMAGINGPANELTRANSLATION_TRANSFORM_NODE_REFERENCE_ROLE = "leftImagingPanelTranslationTransformRef";

static const char* RIGHTIMAGINGPANEL_TO_RIGHTIMAGINGPANELORIGIN_TRANSFORM_NODE_REFERENCE_ROLE = "rightImagingPanelToRightImagingPanelTransformRef";
static const char* RIGHTIMAGINGPANELORIGIN_TO_RIGHTIMAGINGPANELROTATED_TRANSFORM_NODE_REFERENCE_ROLE = "rightImagingPanelOriginToRightImagingPanelRotated";
static const char* RIGHTIMAGINGPANELROTATED_TO_GANTRY_TRANSFORM_NODE_REFERENCE_ROLE = "rightImagingPanelRotatedToGantryTransformRef";
static const char* RIGHTIMAGINGPANELTRANSLATION_TRANSFORM_NODE_REFERENCE_ROLE = "rightImagingPanelTranslationTransformRef";

static const char* PATIENTSUPPORT_TO_FIXEDREFERENCE_TRANSFORM_NODE_REFERENCE_ROLE = "patientSupportToFixedReferenceTransformRef";
static const char* PATIENTSUPPORTSCALED_TO_PATIENTSUPPORT_TRANSFORM_NODE_REFERENCE_ROLE = "patientSupportScaledToPatientSupportTransformRef";
static const char* PATIENTSUPPORTSCALEDMOVED_TO_PATIENTSUPPORTCOMPRESS_TRANSFORM_NODE_REFERENCE_ROLE = "patientSupportScaledMovedToPatientSupportCompressTransformRef";

static const char* TABLETOP_TO_TABLETOPECCENTRICROTATION_TRANSFORM_NODE_REFERENCE_ROLE = "tableTopToTableTopEccentricRotationTransformRef";
static const char* TABLETOPECCENTRICROTATION_TO_PATIENTSUPPORT_TRANSFORM_NODE_REFERENCE_ROLE = "tableTopEccentricToPatientSupportTransformRef";
static const char* TABLETOPMOVED_TO_PATIENTSUPPORTMOVED_TRANSFORM_NODE_REFERENCE_ROLE = "tableTopMovedToPatientSupportMovedTransformRef";


//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLRoomsEyeViewNode);

//----------------------------------------------------------------------------
vtkMRMLRoomsEyeViewNode::vtkMRMLRoomsEyeViewNode()
{
  this->GantryRotationAngle = 0.0; //TODO:
  this->CollimatorRotationAngle = 0.0;
  this->ImagingPanelMovement = -68.50;
  this->PatientSupportRotationAngle = 0.0;
  this->VerticalTableTopDisplacement = 0.0;
  this->LongitudinalTableTopDisplacement = 0.0;
  this->LateralTableTopDisplacement = 0.0;
}

//----------------------------------------------------------------------------
vtkMRMLRoomsEyeViewNode::~vtkMRMLRoomsEyeViewNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLRoomsEyeViewNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkIndent indent(nIndent);

  of << indent << " GantryRotationAngle=\"" << this->GantryRotationAngle << "\"";
  of << indent << " CollimatorRotationAngle=\"" << this->CollimatorRotationAngle << "\"";
  of << indent << " ImagingPanelMovement=\"" << this->ImagingPanelMovement << "\"";
  of << indent << " PatientSupportRotationAngle=\"" << this->PatientSupportRotationAngle << "\"";
  of << indent << " VerticalTableTopDisplacement=\"" << this->VerticalTableTopDisplacement << "\"";
  of << indent << " LongitudinalTableTopDisplacement=\"" << this->LongitudinalTableTopDisplacement << "\"";
  of << indent << " LateralTableTopDisplacement=\"" << this->LateralTableTopDisplacement << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLRoomsEyeViewNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName = NULL;
  const char* attValue = NULL;

  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "GantryRotationAngle")) 
      {
      this->GantryRotationAngle = vtkVariant(attValue).ToDouble();
      }
    //else if... TODO:
    else if (!strcmp(attName, "CollimatorRotationAngle"))
    {
      this->CollimatorRotationAngle = vtkVariant(attValue).ToDouble();
    }
    else if (!strcmp(attName, "ImagingPanelMovement"))
    {
      this->ImagingPanelMovement = vtkVariant(attValue).ToDouble();
    }
    else if (!strcmp(attName, "PatientSupportRotationAngle"))
    {
      this->PatientSupportRotationAngle = vtkVariant(attValue).ToDouble();
    }
    else if (!strcmp(attName, "VerticalTableTopDisplacement"))
    {
      this->VerticalTableTopDisplacement = vtkVariant(attValue).ToDouble();
    }
    else if (!strcmp(attName, "LongitudinalTableTopDisplacement"))
    {
      this->LongitudinalTableTopDisplacement = vtkVariant(attValue).ToDouble();
    }
    else if (!strcmp(attName, "LateralTableTopDisplacement"))
    {
      this->LateralTableTopDisplacement = vtkVariant(attValue).ToDouble();
    }
    }

  // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLRoomsEyeViewNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  this->DisableModifiedEventOn();

  vtkMRMLRoomsEyeViewNode *node = (vtkMRMLRoomsEyeViewNode *) anode;

  this->GantryRotationAngle = node->GantryRotationAngle;
  this->CollimatorRotationAngle = node->CollimatorRotationAngle;
  this->ImagingPanelMovement = node->ImagingPanelMovement;
  this->PatientSupportRotationAngle = node->PatientSupportRotationAngle;
  this->VerticalTableTopDisplacement = node->VerticalTableTopDisplacement;
  this->LongitudinalTableTopDisplacement = node->LongitudinalTableTopDisplacement;
  this->LateralTableTopDisplacement = node->LateralTableTopDisplacement;

  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLRoomsEyeViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "GantryRotationAngle:   " << this->GantryRotationAngle << "\n";
  os << indent << "CollimatorRotationAngle:   " << this->CollimatorRotationAngle << "\n";
  os << indent << "ImagingPanelMovement:    " << this->ImagingPanelMovement << "\n";
  os << indent << "PatientSupportRotationAngle:   " << this->PatientSupportRotationAngle << "\n";
  os << indent << "VerticalTableTopDisplacement:    " << this->VerticalTableTopDisplacement << "\n";
  os << indent << "LongitudinalTableTopDisplacement:    " << this->LongitudinalTableTopDisplacement << "\n";
  os << indent << "LateralTableTopDisplacement:    " << this->LateralTableTopDisplacement << "\n";
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetGantryToFixedReferenceTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(GANTRY_TO_FIXEDREFERENCE_TRANSFORM_NODE_REFERENCE_ROLE));
}

//----------------------------------------------------------------------------
void vtkMRMLRoomsEyeViewNode::SetAndObserveGantryToFixedReferenceTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(GANTRY_TO_FIXEDREFERENCE_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}
//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetCollimatorToGantryTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(COLLIMATOR_TO_GANTRY_TRANSFORM_NODE_REFERENCE_ROLE));
}

//----------------------------------------------------------------------------
void vtkMRMLRoomsEyeViewNode::SetAndObserveCollimatorToGantryTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(COLLIMATOR_TO_GANTRY_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}
//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetLeftImagingPanelToLeftImagingPanelOriginTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(LEFTIMAGINGPANEL_TO_LEFTIMAGINGPANELORIGIN_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLRoomsEyeViewNode::SetAndObserveLeftImagingPanelToLeftImagingPanelOriginTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(LEFTIMAGINGPANEL_TO_LEFTIMAGINGPANELORIGIN_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetLeftImagingPanelOriginToLeftImagingPanelRotatedTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(LEFTIMAGINGPANELORIGIN_TO_LEFTIMAGINGPANELROTATED_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLRoomsEyeViewNode::SetAndObserveLeftImagingPanelOriginToLeftImagingPanelRotatedTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(LEFTIMAGINGPANELORIGIN_TO_LEFTIMAGINGPANELROTATED_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetLeftImagingPanelRotatedToGantryTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(LEFTIMAGINGPANELROTATED_TO_GANTRY_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLRoomsEyeViewNode::SetAndObserveLeftImagingPanelRotatedToGantryTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(LEFTIMAGINGPANELROTATED_TO_GANTRY_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetLeftImagingPanelTranslationTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(LEFTIMAGINGPANELTRANSLATION_TRANSFORM_NODE_REFERENCE_ROLE));
}
void vtkMRMLRoomsEyeViewNode::SetAndObserveLeftImagingPanelTranslationTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(LEFTIMAGINGPANELTRANSLATION_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}
//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetRightImagingPanelToRightImagingPanelOriginTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(RIGHTIMAGINGPANEL_TO_RIGHTIMAGINGPANELORIGIN_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLRoomsEyeViewNode::SetAndObserveRightImagingPanelToRightImagingPanelOriginTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(RIGHTIMAGINGPANEL_TO_RIGHTIMAGINGPANELORIGIN_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetRightImagingPanelOriginToRightImagingPanelRotatedTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(RIGHTIMAGINGPANELORIGIN_TO_RIGHTIMAGINGPANELROTATED_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLRoomsEyeViewNode::SetAndObserveRightImagingPanelOriginToRightImagingPanelRotatedTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(RIGHTIMAGINGPANELORIGIN_TO_RIGHTIMAGINGPANELROTATED_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetRightImagingPanelRotatedToGantryTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(RIGHTIMAGINGPANELROTATED_TO_GANTRY_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLRoomsEyeViewNode::SetAndObserveRightImagingPanelRotatedToGantryTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(RIGHTIMAGINGPANELROTATED_TO_GANTRY_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetRightImagingPanelTranslationTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(RIGHTIMAGINGPANELTRANSLATION_TRANSFORM_NODE_REFERENCE_ROLE));
}
void vtkMRMLRoomsEyeViewNode::SetAndObserveRightImagingPanelTranslationTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(RIGHTIMAGINGPANELTRANSLATION_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

//-------------------------------------------------------

vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetPatientSupportToFixedReferenceTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(PATIENTSUPPORT_TO_FIXEDREFERENCE_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLRoomsEyeViewNode::SetAndObservePatientSupportToFixedReferenceTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(PATIENTSUPPORT_TO_FIXEDREFERENCE_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));

}

vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetPatientSupportScaledToPatientSupportTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(PATIENTSUPPORTSCALED_TO_PATIENTSUPPORT_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLRoomsEyeViewNode::SetAndObservePatientSupportScaledToPatientSupportTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(PATIENTSUPPORTSCALED_TO_PATIENTSUPPORT_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetPatientSupportScaledMovedToPatientSupportCompressTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(PATIENTSUPPORTSCALEDMOVED_TO_PATIENTSUPPORTCOMPRESS_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLRoomsEyeViewNode::SetAndObservePatientSupportScaledMovedToPatientSupportCompressTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(PATIENTSUPPORTSCALEDMOVED_TO_PATIENTSUPPORTCOMPRESS_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

//-----------------------------------------------------------

vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetTableTopToTableTopEccentricRotationTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(TABLETOP_TO_TABLETOPECCENTRICROTATION_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLRoomsEyeViewNode::SetAndObserveTableTopToTableTopEccentricRotationTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(TABLETOP_TO_TABLETOPECCENTRICROTATION_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetTableTopEccentricRotationToPatientSupportTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(TABLETOPECCENTRICROTATION_TO_PATIENTSUPPORT_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLRoomsEyeViewNode::SetAndObserveTableTopEccentricRotationToPatientSupportTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(TABLETOPECCENTRICROTATION_TO_PATIENTSUPPORT_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

vtkMRMLLinearTransformNode* vtkMRMLRoomsEyeViewNode::GetTableTopMovedToPatientSupportMovedTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(TABLETOPMOVED_TO_PATIENTSUPPORTMOVED_TRANSFORM_NODE_REFERENCE_ROLE));
}

void vtkMRMLRoomsEyeViewNode::SetAndObserveTableMovedToPatientSupportMovedTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNodeReferenceID(TABLETOPMOVED_TO_PATIENTSUPPORTMOVED_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}