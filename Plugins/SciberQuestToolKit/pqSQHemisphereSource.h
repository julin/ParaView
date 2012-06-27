/*
   ____    _ __           ____               __    ____
  / __/___(_) /  ___ ____/ __ \__ _____ ___ / /_  /  _/__  ____
 _\ \/ __/ / _ \/ -_) __/ /_/ / // / -_|_-</ __/ _/ // _ \/ __/
/___/\__/_/_.__/\__/_/  \___\_\_,_/\__/___/\__/ /___/_//_/\__(_)

Copyright 2012 SciberQuest Inc.

*/
#ifndef pqSQHemisphereSource_h
#define pqSQHemisphereSource_h

#include "pqNamedObjectPanel.h"
#include "pqComponentsExport.h"// no comment
#include <vector>// no comment

#include "ui_pqSQHemisphereSourceForm.h"//  no comment
using Ui::pqSQHemisphereSourceForm;

// Define the following to enable debug io
// #define pqSQHemisphereSourceDEBUG

class pqProxy;
class vtkEventQtSlotConnect;
class QWidget;

class pqSQHemisphereSource : public pqNamedObjectPanel
{
  Q_OBJECT
public:
  pqSQHemisphereSource(pqProxy* proxy, QWidget* p = NULL);
  ~pqSQHemisphereSource();


protected slots:
  // Description:
  // read state from disk.
  void Restore();
  void loadConfiguration();
  // Description:
  // write state to disk.
  void Save();
  void saveConfiguration();

  // Description:
  // Update the UI with values from the server.
  void PullServerConfig();
  void PushServerConfig();

  // Description:
  // Update information events are generated by PV in many instances.
  // We need to watch for the ones coresponding to RequestInformation
  // on the server side where the new database view is stored in
  // vtkInformation. This will take that information object
  // and load it in to the QTreeWidget.
  void UpdateInformationEvent();
  // Description:
  // This is where we have to communicate our state to the server.
  void accept();
  //Description:
  // UI driven reset of widget to current server manager values.
  void reset();


private:
  pqSQHemisphereSourceForm *Form;
  vtkEventQtSlotConnect *VTKConnect;
};

#endif
