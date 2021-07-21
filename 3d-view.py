#!/usr/local/bin/pvtkpython
import os
import sys
import vtk

k_data_dir="/home/bibo/works/vtk-s/textbook-example-and-data/Data/headsq/"
k_file_prefix="quarter"

# Start by loading some data.
v16 = vtk.vtkVolume16Reader()
v16.SetDataDimensions(64, 64)
v16.SetDataByteOrderToLittleEndian()
v16.SetFilePrefix(k_data_dir+k_file_prefix)
v16.SetImageRange(1, 93)
v16.SetDataSpacing(3.2, 3.2, 1.5)
v16.Update()

xMin, xMax, yMin, yMax, zMin, zMax = v16.GetExecutive().GetWholeExtent(v16.GetOutputInformation(0))
v16_odata=v16.GetOutput()
print(type(v16_odata), "dir(v16_odata)")
spacing = v16_odata.GetSpacing()
sx, sy, sz = spacing
origin = v16_odata.GetOrigin()
ox, oy, oz = origin

print("v16 data info x,y,z[min, max]: ", xMin, xMax, yMin, yMax, zMin, zMax);
print("v16 spacing:", spacing, "origin:", origin)
#-----------------------------------------------------------------
# An outline is shown for context.
outline = vtk.vtkOutlineFilter()
outline.SetInputConnection(v16.GetOutputPort())

outlineMapper = vtk.vtkPolyDataMapper()
outlineMapper.SetInputConnection(outline.GetOutputPort())

outlineActor = vtk.vtkActor()
outlineActor.SetMapper(outlineMapper)

# The shared picker enables us to use 3 planes at one time
# and gets the picking order right
picker = vtk.vtkCellPicker()
picker.SetTolerance(0.005)

# The 3 image plane widgets are used to probe the dataset.
planeWidgetX = vtk.vtkImagePlaneWidget()
planeWidgetX.DisplayTextOn()
planeWidgetX.SetInputConnection(v16.GetOutputPort())
planeWidgetX.SetPlaneOrientationToXAxes()
planeWidgetX.SetSliceIndex(32)
planeWidgetX.SetPicker(picker)
planeWidgetX.SetKeyPressActivationValue("x")
prop1 = planeWidgetX.GetPlaneProperty()
prop1.SetColor(1, 0, 0)

planeWidgetY = vtk.vtkImagePlaneWidget()
planeWidgetY.DisplayTextOn()
planeWidgetY.SetInputConnection(v16.GetOutputPort())
planeWidgetY.SetPlaneOrientationToYAxes()
planeWidgetY.SetSliceIndex(32)
planeWidgetY.SetPicker(picker)
planeWidgetY.SetKeyPressActivationValue("y")
prop2 = planeWidgetY.GetPlaneProperty()
prop2.SetColor(1, 1, 0)
planeWidgetY.SetLookupTable(planeWidgetX.GetLookupTable())

# for the z-slice, turn off texture interpolation:
# interpolation is now nearest neighbour, to demonstrate
# cross-hair cursor snapping to pixel centers
planeWidgetZ = vtk.vtkImagePlaneWidget()
planeWidgetZ.DisplayTextOn()
planeWidgetZ.SetInputConnection(v16.GetOutputPort())
planeWidgetZ.SetPlaneOrientationToZAxes()
planeWidgetZ.SetSliceIndex(46)
planeWidgetZ.SetPicker(picker)
planeWidgetZ.SetKeyPressActivationValue("z")
prop3 = planeWidgetZ.GetPlaneProperty()
prop3.SetColor(0, 0, 1)
planeWidgetZ.SetLookupTable(planeWidgetX.GetLookupTable())

# Create the RenderWindow and Renderer
ren = vtk.vtkRenderer()
renWin = vtk.vtkRenderWindow()
renWin.AddRenderer(ren)

# Add the outline actor to the renderer, set the background color and size
ren.AddActor(outlineActor)
renWin.SetSize(600, 600)
ren.SetBackground(0.1, 0.1, 0.2)

current_widget = planeWidgetZ
mode_widget = planeWidgetZ

#:help CTRL-V-alternative 
#Set the interactor for the widgets
iact = vtk.vtkRenderWindowInteractor()
iact.SetRenderWindow(renWin)

style = vtk.vtkInteractorStyleTrackballCamera()  #vtkInteractorStyle()
iact.SetInteractorStyle(style)

planeWidgetX.SetInteractor(iact)
planeWidgetX.On()
planeWidgetY.SetInteractor(iact)
planeWidgetY.On()
planeWidgetZ.SetInteractor(iact)
planeWidgetZ.On()
# Create an initial interesting view
ren.ResetCamera()
cam1 = ren.GetActiveCamera()
cam1.Elevation(110)
cam1.SetViewUp(0, 0, -1)
cam1.Azimuth(45)
ren.ResetCameraClippingRange()
iact.Initialize()
#renWin.Render()
iact.Start()
#============================================
if "__main__" == __name__:
    print("using pvtkPython:starting 3d view")
