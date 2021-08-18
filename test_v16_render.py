#!/usr/local/bin/pvtkpython
import os
import sys
import vtk
import copy

k_data_dir = "/home/bibo/works/vtk-s/textbook-example-and-data/Data/headsq/"
k_file_prefix = "quarter"


def print_cam_info(render):
    # Create an initial interesting view
    render.ResetCamera()
    cam1 = render.GetActiveCamera()
    cam1.Elevation(110)
    cam1.SetViewUp(0, 0, -1)
    cam1.Azimuth(45)
    render.ResetCameraClippingRange()


def get_point_obj(x=0.0, y=0.0, z=0.0, c='red'):
    colors = vtk.vtkNamedColors()
    # Create the geometry of a point (the coordinate)
    points = vtk.vtkPoints()
    p = [x, y, z]

    # We need an an array of point id's for InsertNextCell.
    pid = [None]
    if points.GetNumberOfPoints() < 1:
        pid[0] = points.InsertNextPoint(p)
    else:
        points.SetPoint(pid[0], p)
    # Create the topology of the point (a vertex)
    vertices = vtk.vtkCellArray()
    vertices.InsertNextCell(1, pid)

    # Create a polydata object
    pointPD = vtk.vtkPolyData()
    # Set the points and vertices we created as the geometry and topology of the polydata
    pointPD.SetPoints(points)
    pointPD.SetVerts(vertices)

    # Visualize
    pointMapper = vtk.vtkPolyDataMapper()
    pointMapper.SetInputData(pointPD)

    pointActor = vtk.vtkActor()
    pointActor.SetMapper(pointMapper)
    pointActor.GetProperty().SetColor(colors.GetColor3d(c))
    pointActor.GetProperty().SetPointSize(9)
    return pointActor


def main(argv):
    colors = vtk.vtkNamedColors()
    # Start by loading some data.
    v16 = vtk.vtkVolume16Reader()
    v16.SetDataDimensions(64, 64)
    v16.SetDataByteOrderToLittleEndian()
    v16.SetFilePrefix(k_data_dir + k_file_prefix)
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

    pointAct1 = get_point_obj(0.0, 0.0, 0.0, 'white')
    pointAct2 = get_point_obj(201.6, 201.6, 138.0, 'yellow')
    pointActx = get_point_obj(201.6, 0, 0.0, 'red')
    pointActy = get_point_obj(0.6, 201.6, 0.0, 'green')
    pointActz = get_point_obj(0.6, 0.6, 138.0, 'blue')
    ren = vtk.vtkRenderer()
    ren.AddActor(outlineActor)
    ren.AddActor(pointAct1)
    ren.AddActor(pointAct2)
    ren.AddActor(pointActx)
    ren.AddActor(pointActy)
    ren.AddActor(pointActz)
    ren.SetBackground(colors.GetColor3d('purple'))
    #ren.SetViewport(0.51, 0.0, 1.0, 0.49)

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
    normal = imgPlaneWidget.GetNormal()
    print("ipw-normal=", normal)
    if 1:
        imgPlaneWidget.SetPlaneOrientationToXAxes()
        print("after set 2X, normal=", imgPlaneWidget.GetNormal())

        imgPlaneWidget.SetPlaneOrientationToYAxes()
        print("after set 2Y, normal=", imgPlaneWidget.GetNormal())

        imgPlaneWidget.SetPlaneOrientationToZAxes()
        print("after set 2z, normal=", imgPlaneWidget.GetNormal())
        #imgPlaneWidget.SetKeyPressActivationValue("x")
        #imgPlaneWidget.SetResliceInterpolateToNearestNeighbour() # enumerate=0

    #print("camera info:", ren.GetActiveCamera())
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