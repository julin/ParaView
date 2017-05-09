/*=========================================================================

  Program:   ParaView
  Module:    vtkPVCylinder

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkPVCylinder
 * @brief   extends vtkCylinder to add ParaView specific API.
 *
 * vtkPVCylinder extends vtkCylinder to add ParaView specific API.
*/

#ifndef vtkPVCylinder_h
#define vtkPVCylinder_h

#include "vtkCylinder.h"
#include "vtkPVVTKExtensionsDefaultModule.h" //needed for exports

class VTKPVVTKEXTENSIONSDEFAULT_EXPORT vtkPVCylinder : public vtkCylinder
{
public:
  static vtkPVCylinder* New();

  vtkTypeMacro(vtkPVCylinder, vtkCylinder);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  //@{
  /**
   * Get/Set the vector defining the direction of the cylinder.
   */
  void SetOrientedAxis(double x, double y, double z);
  void SetOrientedAxis(const double axis[3]);
  vtkGetVector3Macro(OrientedAxis, double);
  //@}

  // Reimplemented to update transform on change:
  virtual void SetCenter(double x, double y, double z) VTK_OVERRIDE;
  virtual void SetCenter(double xyz[3]) VTK_OVERRIDE;

protected:
  vtkPVCylinder();
  ~vtkPVCylinder();

  void UpdateTransform();

  double OrientedAxis[3];

private:
  vtkPVCylinder(const vtkPVCylinder&) VTK_DELETE_FUNCTION;
  void operator=(const vtkPVCylinder&) VTK_DELETE_FUNCTION;
};

inline void vtkPVCylinder::SetOrientedAxis(double x, double y, double z)
{
  double axis[3] = { x, y, z };
  this->SetOrientedAxis(axis);
}

#endif
