    if {[info exists argc]} { 
        set argcm1 [expr $argc - 1]
        for {set i 0} {$i < $argcm1} {incr i} {
            if {[lindex $argv $i] == "-D" && $i < $argcm1} {
		set DataDir [lindex $argv [expr $i + 1]]
            }
        }
    } 
proc AdjustContourValue { cnt val } {
    set entry [$cnt GetPVWidget {Contour Values}]
    $entry RemoveAllValues
    $entry AddValue $val
    $entry AddValueCallback
    $cnt AcceptCallback
}




# hack to get the window.
set pvWindow [lindex [vtkPVWindow ListInstances] 0]

# Create the reader
set pInt [$pvWindow GetSourceInterface "vtkPLOT3DReader"]
set pvPlot3D [$pInt CreateCallback]
[$pvPlot3D GetVTKSource] SetXYZFileName $DataDir/Data/combxyz.bin
[$pvPlot3D GetVTKSource] SetQFileName $DataDir/Data/combq.bin
$pvPlot3D UpdateParameterWidgets
$pvPlot3D AcceptCallback

# Create the contour filter
set cInt [$pvWindow ContourCallback]
$cInt AcceptCallback

$pvWindow SetCurrentPVSource $pvPlot3D

set eInt [$pvWindow GetSourceInterface "vtkExtractGrid"]
set pvExtract [$eInt CreateCallback]

[$pvExtract GetVTKSource] SetVOI 0 56 0 32 0 0 
$pvExtract UpdateParameterWidgets
$pvExtract AcceptCallback

[$pvExtract GetPVOutput] SetVisibility 0

set pvGlyph [$pvWindow GlyphCallback]
$pvGlyph AcceptCallback

[$pvGlyph GetPVOutput] ColorByProperty
[$pvGlyph GetVTKSource] SetScaleFactor 3.0
$pvGlyph UpdateParameterWidgets
$pvGlyph AcceptCallback

[$pvGlyph GetPVOutput] SetVisibility 0


set cntOutput [$cInt GetPVOutput]

set camera [Ren1 GetActiveCamera]
$camera SetPosition -29.1076 -40.3242 38.8874
$camera SetFocalPoint 8.255 0 29.7631
$camera SetViewUp -0.00809309 0.227813 0.973671
$camera SetViewAngle 30
$camera SetClippingRange 34.7159 82.3051

# Vary isosuface
$cntOutput SetVisibility 1
[$pvPlot3D GetPVOutput] SetVisibility 1

RenWin1 SetSize 300 300

set start 0.2
set end 0.7
set num 30
set incr [expr ($end - $start)/$num]
for {set i 0} {$i < $num} {incr i 1} {
    set val [expr $start + $i * $incr]
    AdjustContourValue $cInt $val 

}

# Rotate isosuface
AdjustContourValue $cInt 0.3

$cntOutput SetScalarBarVisibility 1
$cntOutput SetScalarBarOrientationToHorizontal
$cntOutput ColorByPointFieldComponent Momentum 0
$cInt UpdateParameterWidgets

RenWin1 SetSize 300 300

for {set i 0} {$i < 102} {incr i 1} {
    $camera Azimuth 10
    RenWin1 Render
    update

}

$cntOutput SetVisibility 0
$cntOutput SetScalarBarVisibility 0

# Here goes cut-plane (see end of this file)

$camera SetPosition -28.8139 1.07506 33.8794
$camera SetFocalPoint 26.5733 -0.117269 27.8723
$camera SetViewUp 0.10793 0.00506272 0.994146
$camera SetViewAngle 30
$camera SetClippingRange 18.5955 61.8423

[$pvExtract GetVTKSource] SetVOI 0 56 0 32 0 0 
$pvExtract UpdateParameterWidgets
$pvExtract AcceptCallback

[$pvExtract GetPVOutput] SetVisibility 1


RenWin1 Render

for {set i 0} {$i < 24} {incr i 1} {
    [$pvExtract GetVTKSource] SetVOI 0 56 0 32 $i $i
    $pvExtract UpdateParameterWidgets
    $pvExtract AcceptCallback
    RenWin1 Render
    update

}

for {set i 23} {$i >= 0} {incr i -1} {
    [$pvExtract GetVTKSource] SetVOI 0 56 0 32 $i $i
    $pvExtract UpdateParameterWidgets
    $pvExtract AcceptCallback
    RenWin1 Render
    update

}

[$pvGlyph GetPVOutput] SetVisibility 1

$camera SetPosition 36.8437 -0.196199 53.7392
$camera SetFocalPoint -5.92921 -0.324064 18.022
$camera SetViewUp -0.639592 -0.0625532 0.766165
$camera SetViewAngle 30
$camera SetClippingRange 15.0904 64.6074

[$pvExtract GetVTKSource] SetVOI 0 0 0 32 3 20
RenWin1 Render

for {set i 1} {$i < 56} {incr i 1} {
    [$pvExtract GetVTKSource] SetVOI $i $i 0 32 3 20
    $pvExtract UpdateParameterWidgets
    $pvExtract AcceptCallback
    RenWin1 Render
    update

}

source $DataDir/Utility/rtImage.tcl
pvImageTest $DataDir/Baseline/CombustorTest.png 10

Application Exit
