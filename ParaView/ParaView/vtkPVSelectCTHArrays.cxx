/*=========================================================================

  Program:   ParaView
  Module:    vtkPVSelectCTHArrays.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPVSelectCTHArrays.h"

#include "vtkCollection.h"
#include "vtkKWCheckButton.h"
#include "vtkKWLabel.h"
#include "vtkKWListBox.h"
#include "vtkKWPushButton.h"
#include "vtkKWWidget.h"
#include "vtkObjectFactory.h"
#include "vtkPVApplication.h"
#include "vtkPVArrayInformation.h"
#include "vtkPVData.h"
#include "vtkPVDataInformation.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkPVInputMenu.h"
#include "vtkPVPart.h"
#include "vtkPVProcessModule.h"
#include "vtkPVSource.h"
#include "vtkPVXMLElement.h"
#include "vtkStringList.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPVSelectCTHArrays);
vtkCxxRevisionMacro(vtkPVSelectCTHArrays, "1.8");
vtkCxxSetObjectMacro(vtkPVSelectCTHArrays, InputMenu, vtkPVInputMenu);

int vtkPVSelectCTHArraysCommand(ClientData cd, Tcl_Interp *interp,
                                int argc, char *argv[]);

//----------------------------------------------------------------------------
vtkPVSelectCTHArrays::vtkPVSelectCTHArrays()
{
  this->CommandFunction = vtkPVSelectCTHArraysCommand;
  
  this->ButtonFrame = vtkKWWidget::New();
  this->ShowAllLabel = vtkKWLabel::New();
  this->ShowAllCheck = vtkKWCheckButton::New();

  this->ArraySelectionList = vtkKWListBox::New();
  this->ArrayLabelCollection = vtkCollection::New();
  this->SelectedArrayNames = vtkStringList::New();

  this->InputMenu = NULL;
  this->Active = 1;
}

//----------------------------------------------------------------------------
vtkPVSelectCTHArrays::~vtkPVSelectCTHArrays()
{
  this->ButtonFrame->Delete();
  this->ButtonFrame = NULL;
  this->ShowAllLabel->Delete();
  this->ShowAllLabel = NULL;
  this->ShowAllCheck->Delete();
  this->ShowAllCheck = NULL;

  this->ArraySelectionList->Delete();
  this->ArraySelectionList = NULL;
  this->ArrayLabelCollection->Delete();
  this->ArrayLabelCollection = NULL;
  this->SelectedArrayNames->Delete();
  this->SelectedArrayNames = NULL;

  this->SetInputMenu(NULL);
}

//----------------------------------------------------------------------------
void vtkPVSelectCTHArrays::Create(vtkKWApplication *app)
{
  vtkPVApplication* pvApp = vtkPVApplication::SafeDownCast(app);

  if (this->Application)
    {
    vtkErrorMacro("PVWidget already created");
    return;
    }
  this->SetApplication(app);

  // create the top level
  this->Script("frame %s -borderwidth 0 -relief flat", this->GetWidgetName());

  
  this->ButtonFrame->SetParent(this);
  this->ButtonFrame->Create(pvApp, "frame", "");
  this->Script("pack %s -side top -fill x",
               this->ButtonFrame->GetWidgetName());
  this->ShowAllLabel->SetParent(this->ButtonFrame);
  this->ShowAllLabel->Create(pvApp, "");
  this->ShowAllLabel->SetLabel("Show All");
  this->ShowAllCheck->SetParent(this->ButtonFrame);
  this->ShowAllCheck->Create(pvApp, "");
  this->ShowAllCheck->SetState(0);
  this->ShowAllCheck->SetCommand(this, "ShowAllArraysCheckCallback");

  this->ShowAllCheck->SetBalloonHelpString("Hide arrays that are not called 'Volume Fraction'");


  this->Script("pack %s %s -side left -fill x -expand t",
               this->ShowAllLabel->GetWidgetName(),
               this->ShowAllCheck->GetWidgetName());

  this->ArraySelectionList->SetParent(this);
  this->ArraySelectionList->ScrollbarOff();
  this->ArraySelectionList->Create(app, "-selectmode extended");
  this->ArraySelectionList->SetHeight(0);
  // I assume we need focus for control and alt modifiers.
  this->Script("bind %s <Enter> {focus %s}",
               this->ArraySelectionList->GetWidgetName(),
               this->ArraySelectionList->GetWidgetName());

  this->Script("pack %s -side top -fill both -expand t",
               this->ArraySelectionList->GetWidgetName());
  this->ArraySelectionList->SetBalloonHelpString("Select parts to extract. Use control key for toggling selection. Use shift key for extended selection");

  // There is no current way to get a modified call back, so assume
  // the user will change the list.  This widget will only be used once anyway.
  this->ModifiedCallback();
}


//----------------------------------------------------------------------------
void vtkPVSelectCTHArrays::Inactivate()
{
  const char* arrayName;
  int num, idx;
  vtkKWLabel* label;

  this->Active = 0;

  this->Script("pack forget %s %s", this->ButtonFrame->GetWidgetName(),
               this->ArraySelectionList->GetWidgetName());

  this->SelectedArrayNames->RemoveAllItems();
  num = this->ArraySelectionList->GetNumberOfItems();
  for (idx = 0; idx < num; ++idx)
    {
    if (this->ArraySelectionList->GetSelectState(idx))
      {
      arrayName = this->ArraySelectionList->GetItem(idx);
      this->SelectedArrayNames->AddString(arrayName);
      label = vtkKWLabel::New();
      label->SetParent(this);
      label->SetLabel(arrayName);
      label->Create(this->Application, "");
      this->Script("pack %s -side top -anchor w",
                   label->GetWidgetName());
      this->ArrayLabelCollection->AddItem(label);
      label->Delete();
      }
    }
}

//----------------------------------------------------------------------------
void vtkPVSelectCTHArrays::AcceptInternal(vtkClientServerID vtkSourceID)
{
  int num, idx;
  const char* arrayName;
  int state;

  if ( ! this->Active)
    {
    return;
    }

  num = this->ArraySelectionList->GetNumberOfItems();

  vtkPVApplication *pvApp = this->GetPVApplication();

  if (this->ModifiedFlag)
    {
    this->Inactivate();
    }

  vtkPVProcessModule* pm = pvApp->GetProcessModule();
  pm->GetStream() << vtkClientServerStream::Invoke <<  vtkSourceID
                  << "RemoveAllVolumeArrayNames"
                  << vtkClientServerStream::End;
  pm->SendStreamToServer();

  // Now loop through the input mask setting the selection states.
  for (idx = 0; idx < num; ++idx)
    {
    state = this->ArraySelectionList->GetSelectState(idx);
    if (state)
      {
      arrayName = this->ArraySelectionList->GetItem(idx); 
      pm->GetStream() << vtkClientServerStream::Invoke <<  vtkSourceID
                      << "AddVolumeArrayName"
                      << arrayName
                      << vtkClientServerStream::End;   
      pm->SendStreamToServer();
      }
    }

  this->ModifiedFlag = 0;
}


//---------------------------------------------------------------------------
void vtkPVSelectCTHArrays::ClearAllSelections()
{
  int idx, num;

  if ( ! this->Active)
    {
    vtkErrorMacro("Trying to change the selection of an inactive widget.");
    return;
    }

  num = this->ArraySelectionList->GetNumberOfItems();
  for (idx = 0; idx < num; ++idx)
    {
    this->ArraySelectionList->SetSelectState(idx, 0);
    }  
}

//---------------------------------------------------------------------------
void vtkPVSelectCTHArrays::SetSelectState(const char* arrayName, int val)
{
  int idx, num;
  const char* listArrayName;

  if ( ! this->Active)
    {
    vtkErrorMacro("Trying to change the selection of an inactive widget.");
    return;
    }

  num = this->ArraySelectionList->GetNumberOfItems();
  for (idx = 0; idx < num; ++idx)
    {
    listArrayName = this->ArraySelectionList->GetItem(idx);
    if (strcmp(arrayName,listArrayName) == 0)
      {
      this->ArraySelectionList->SetSelectState(idx, val);
      return;
      }
    }

  vtkErrorMacro("Could not find array with name " << arrayName);
}


//---------------------------------------------------------------------------
void vtkPVSelectCTHArrays::Trace(ofstream *file)
{
  int num, idx;
  const char* arrayName;

  if ( ! this->InitializeTrace(file))
    {
    return;
    }

  *file << "$kw(" << this->GetTclName() << ") ClearAllSelections\n";

  num = this->SelectedArrayNames->GetNumberOfStrings();
  for (idx = 0; idx < num; ++idx)
    {
    arrayName = this->SelectedArrayNames->GetString(idx);
      *file << "$kw(" << this->GetTclName() << ") SetSelectState {"
            << arrayName << "} 1\n"; 
    }
}

//----------------------------------------------------------------------------
void vtkPVSelectCTHArrays::ResetInternal()
{
  if (this->Active)
    {
    this->ArraySelectionList->DeleteAll();
    this->Update();
    }
}

//----------------------------------------------------------------------------
void vtkPVSelectCTHArrays::Update()
{
  int showAll = this->ShowAllCheck->GetState();
  int volumeFlag;
  int voidFlag;
  int num, idx;
  vtkPVDataSetAttributesInformation* attrInfo;
  vtkPVArrayInformation* arrayInfo;

  if ( ! this->Active)
    {
    return;
    }

  this->ArraySelectionList->DeleteAll();
  if (this->InputMenu == NULL)
    {
    return;
    }    

  attrInfo = this->InputMenu->GetCurrentValue()->GetDataInformation()->GetCellDataInformation();

  num = attrInfo->GetNumberOfArrays();
  int count = 0;
  for (idx = 0; idx < num; ++idx)
    {
    arrayInfo = attrInfo->GetArrayInformation(idx);
    if (arrayInfo->GetNumberOfComponents() == 1)
      {
      volumeFlag = this->StringMatch(arrayInfo->GetName());
      voidFlag = 0;
      if (strncmp(arrayInfo->GetName(), "Void", 4) == 0
          || strncmp(arrayInfo->GetName(), "void", 4) == 0)
        {
        voidFlag = 1;
        }
      if (showAll || this->StringMatch(arrayInfo->GetName()))
        {
        this->ArraySelectionList->InsertEntry(count, arrayInfo->GetName());
        // It would be nice to get rid of the void volume fraction.
        if (volumeFlag && ! voidFlag)
          {
          this->ArraySelectionList->SetSelectState(count, 1);
          }
        ++count;
        }
      }
    }
  // Update now clears selection.
  // We could try to restore the selections.
  // Now loop through the input mask setting the selection states.
  //for (idx = 0; idx < num; ++idx)
  //  {
  //  this->Script("%s SetSelectState %d [%s GetInputMask %d]",
  //               this->PartSelectionList->GetTclName(),
  //               idx, vtkSourceTclName, idx);
  //  }
  // Because list box does not notify us when it is modified ...
  //this->ModifiedFlag = 0;

}

//----------------------------------------------------------------------------
void vtkPVSelectCTHArrays::ShowAllArraysCheckCallback()
{
  this->Update();
}

//----------------------------------------------------------------------------
int vtkPVSelectCTHArrays::StringMatch(const char* arrayName)
{
  const char* p;
  p = arrayName;

  while (*p != '\0')
    {
    if (strncmp(p,"Fraction",8) == 0 || strncmp(p, "fraction", 8) == 0)
      {
      return 1;
      }
    ++p;
    }
  return 0;
}

//----------------------------------------------------------------------------
// Multiple input filter has only one VTK source.
void vtkPVSelectCTHArrays::SaveInBatchScript(ofstream *file)
{
  const char* arrayName;
  int num, idx;

  num = this->SelectedArrayNames->GetNumberOfStrings();

  // Now loop through the input mask setting the selection states.
  for (idx = 0; idx < num; ++idx)
    {
    arrayName = this->SelectedArrayNames->GetString(idx);
    *file << "\tpvTemp" << this->PVSource->GetVTKSourceID(0) 
          << " AddVolumeArrayName {" << arrayName << "}\n";  
    }
}


//----------------------------------------------------------------------------
void vtkPVSelectCTHArrays::CopyProperties(vtkPVWidget* clone, vtkPVSource* pvSource,
                              vtkArrayMap<vtkPVWidget*, vtkPVWidget*>* map)
{
  this->Superclass::CopyProperties(clone, pvSource, map);
  vtkPVSelectCTHArrays* pvsa = vtkPVSelectCTHArrays::SafeDownCast(clone);
  if (pvsa)
    {
    if (this->InputMenu)
      {
      // This will either clone or return a previously cloned
      // object.
      vtkPVInputMenu* im = this->InputMenu->ClonePrototype(pvSource, map);
      pvsa->SetInputMenu(im);
      im->Delete();
      }
    }
  else 
    {
    vtkErrorMacro("Internal error. Could not downcast clone to PVSelectCTHArrays.");
    }
}

//----------------------------------------------------------------------------
int vtkPVSelectCTHArrays::ReadXMLAttributes(vtkPVXMLElement* element,
                                            vtkPVXMLPackageParser* parser)
{
  if (!this->Superclass::ReadXMLAttributes(element, parser)) { return 0; }
    
  // Setup the InputMenu.
  const char* input_menu = element->GetAttribute("input_menu");
  if (input_menu)
    {
    vtkPVXMLElement* ime = element->LookupElement(input_menu);
    vtkPVWidget* w = this->GetPVWidgetFromParser(ime, parser);
    vtkPVInputMenu* imw = vtkPVInputMenu::SafeDownCast(w);
    if(!imw)
      {
      if(w) { w->Delete(); }
      vtkErrorMacro("Couldn't get InputMenu widget " << input_menu);
      return 0;
      }
    imw->AddDependent(this);
    this->SetInputMenu(imw);
    imw->Delete();
    }
  
  return 1;
}

//----------------------------------------------------------------------------
void vtkPVSelectCTHArrays::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "InputMenu: " << this->InputMenu << endl;
}
