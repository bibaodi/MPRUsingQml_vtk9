#!/usr/local/bin/pvtkpython
import os
import sys
import vtk
import copy

k_data_dir="/home/bibo/works/vtk-s/textbook-example-and-data/Data/headsq/"
k_file_prefix="quarter"

def main(argv):
        colors = vtk.vtkNamedColors()
        # Start by loading some data.
        v16 = vtk.vtkVolume16Reader()
        v16.SetDataDimensions(64, 64)
        v16.SetDataByteOrderToLittleEndian()
        v16.SetFilePrefix(k_data_dir+k_file_prefix)
        v16.SetImageRange(1, 93)
        v16.SetDataSpacing(3.2, 3.2, 1.5)
        v16.Update()

        outlineMapper = vtk.vtkPolyDataMapper()
        # An outline is shown for context.
        outline = vtk.vtkOutlineFilter()
        outline.SetInputConnection(v16.GetOutputPort())
        outlineMapper.SetInputConnection(outline.GetOutputPort())
        #outlineMapper.SetInputConnection(v16.GetOutputPort())

        outlineActor = vtk.vtkActor()
        outlineActor.SetMapper(outlineMapper)

        ren = vtk.vtkRenderer()
        ren.AddActor(outlineActor)
        #ren.SetBackground(colors.GetColor3d('violet_dark'))
        ren.SetViewport(0.51, 0.0, 1.0, 0.49) 

        #window
        renWin = vtk.vtkRenderWindow()
        renWin.SetSize(600, 600)
        renWin.AddRenderer(ren)
        
        #interactor
        iact = vtk.vtkRenderWindowInteractor()
        iact.SetRenderWindow(renWin)
        #add IPW
        imgPlaneWidget = vtk.vtkImagePlaneWidget()
        imgPlaneWidget.DisplayTextOn()
        imgPlaneWidget.SetInputConnection(v16.GetOutputPort())
        #imgPlaneWidget.SetPicker(picker)
        #imgPlaneWidget.SetPlaneOrientationToXAxes()
        #imgPlaneWidget.SetKeyPressActivationValue("x")
        #imgPlaneWidget.SetResliceInterpolateToNearestNeighbour() # enumerate=0
        imgPlaneWidget.SetSliceIndex(32)
        imgPlaneWidget.SetCurrentRenderer(ren)
        imgPlaneWidget.SetInteractor(iact)
        imgPlaneWidget.On()
        if 0:
            for i in range(0, 10):
                # Render the image
                renWin.Render()
        else:
            iact.Initialize()
            iact.Start()

    #============================================
if "__main__" == __name__:
    print("using pvtkPython:starting 3d view")
    main(sys.argv)
    print("process end~~~")