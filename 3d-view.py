#!/usr/local/bin/pvtkpython
import os
import sys
import vtk
import copy

k_data_dir="/home/bibo/works/vtk-s/textbook-example-and-data/Data/headsq/"
k_file_prefix="quarter"
def get_cone_mapper():
        cone = vtk.vtkConeSource()
        cone.SetHeight(3.0)
        cone.SetRadius(1.0)
        cone.SetResolution(10)
        coneMapper = vtk.vtkPolyDataMapper()
        coneMapper.SetInputConnection(cone.GetOutputPort())
        return coneMapper

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

        xMin, xMax, yMin, yMax, zMin, zMax = v16.GetExecutive().GetWholeExtent(v16.GetOutputInformation(0))
        v16_odata=v16.GetOutput()
        print(type(v16_odata), "dir(v16_odata)")
        spacing = v16_odata.GetSpacing()
        sx, sy, sz = spacing
        origin = v16_odata.GetOrigin()
        ox, oy, oz = origin
        extent = v16_odata.GetExtent()
        #v16 spacing: (3.2, 3.2, 1.5)
        #origin: (0.0, 0.0, 0.0)
        #extent: (0, 63, 0, 63, 0, 92)
        print("v16 GetWholeExtent info x,y,z[min, max]: ", xMin, xMax, yMin, yMax, zMin, zMax)
        print("v16 spacing:", spacing, "\norigin:", origin, "\nextent:", extent)
        #-----------------------------------------------------------------
        def create_reslice():# reslice it no need now for MPR
                reslice4 = vtk.vtkImageReslice()
                reslice4.SetInputConnection(v16.GetOutputPort())
                reslice4.SetInterpolationModeToLinear()
                reslice4.SetOutputSpacing(3.2,3.2,1.5)
                reslice4.SetOutputOrigin(0,0,0)
                reslice4.SetOutputExtent(0,63,0,63,0,0)

                mapper4 = vtk.vtkImageMapper()
                mapper4.SetInputConnection(reslice4.GetOutputPort())
                mapper4.SetColorWindow(2000)
                mapper4.SetColorLevel(1000)
                mapper4.SetZSlice(0)

                actor4 = vtk.vtkActor2D()
                actor4.SetMapper(mapper4)

        #--------------------------------
        outlineMapper = vtk.vtkPolyDataMapper()
        if 1:
                # An outline is shown for context.
                outline = vtk.vtkOutlineFilter()
                outline.SetInputConnection(v16.GetOutputPort())
                outlineMapper.SetInputConnection(outline.GetOutputPort())
        else:
                outlineMapper.SetInputConnection(v16.GetOutputPort())

        outlineActor = vtk.vtkActor()
        outlineActor.SetMapper(outlineMapper)
        planeActor = vtk.vtkActor()
        planeActor.SetMapper(get_cone_mapper())

        # The shared picker enables us to use 3 planes at one time
        # and gets the picking order right
        picker = vtk.vtkCellPicker()
        picker.SetTolerance(0.005)

        # The 3 image plane widgets are used to probe the dataset.
        def get_planeWidget_instance(orientation='x'):
                imgPlaneWidget = vtk.vtkImagePlaneWidget()
                imgPlaneWidget.DisplayTextOn()
                imgPlaneWidget.SetInputConnection(v16.GetOutputPort())
                imgPlaneWidget.SetSliceIndex(32)
                imgPlaneWidget.SetPicker(picker)
                prop_color=(1, 0, 0)
                if 'x' == orientation.lower():
                        imgPlaneWidget.SetPlaneOrientationToXAxes()
                        imgPlaneWidget.SetKeyPressActivationValue("x")
                        prop_color=(1, 0, 0)
                elif 'y' == orientation.lower():
                        imgPlaneWidget.SetPlaneOrientationToYAxes()
                        imgPlaneWidget.SetKeyPressActivationValue("y")
                        prop_color=(1, 1, 0)
                elif 'z' == orientation.lower():
                        imgPlaneWidget.SetPlaneOrientationToZAxes()
                        imgPlaneWidget.SetKeyPressActivationValue("z")
                        prop_color=(0, 0, 1)
                prop1 = imgPlaneWidget.GetPlaneProperty()
                prop1.SetColor(*prop_color)
                return imgPlaneWidget

        planeWidgetX = get_planeWidget_instance('x')
        planeWidgetX.SetSliceIndex(32)
        
        planeWidgetY = get_planeWidget_instance('y')
        planeWidgetY.SetSliceIndex(32)

        #planeWidgetY.SetLookupTable(planeWidgetX.GetLookupTable())

        # for the z-slice, turn off texture interpolation:
        # interpolation is now nearest neighbour, to demonstrate
        # cross-hair cursor snapping to pixel centers
        planeWidgetZ = get_planeWidget_instance('z')
        planeWidgetZ.SetSliceIndex(46)
        planeWidgetZ.SetLookupTable(planeWidgetX.GetLookupTable())


        def create_3_imgPlaneWidgets(option=0):
                planeWidgetA = get_planeWidget_instance('x')
                planeWidgetA.SetSliceIndex(32)
                planeWidgetC = get_planeWidget_instance('y')
                planeWidgetC.SetSliceIndex(32)
                planeWidgetT = get_planeWidget_instance('z')
                planeWidgetT.SetSliceIndex(46)
                return [planeWidgetA, planeWidgetC, planeWidgetT]

        # Create the RenderWindow and Renderer
        ren = vtk.vtkRenderer()
        ren2 = vtk.vtkRenderer()
        ren3 = vtk.vtkRenderer()
        ren4 = vtk.vtkRenderer()
        renWin = vtk.vtkRenderWindow()
        renWin.SetSize(600, 600)

        # Add the outline actor to the renderer, set the background color and size
        ren.AddActor(outlineActor)
        ren.SetBackground(colors.GetColor3d('violet_dark'))
        ren.SetViewport(0.51, 0.0, 1.0, 0.49)  # Coordinates are expressed as (xmin,ymin,xmax,ymax), where each coordinate is 0 <= coordinate <= 1.0.

        # config 2nd render
        #ren2.AddActor(planeActor)
        ren2.SetBackground(colors.GetColor3d('Tomato'))
        ren2.SetViewport(0.0, 0.51, 0.49, 1.0)

        # config 3rd render right-top
        #ren3.AddActor(planeActor)
        ren3.SetBackground(colors.GetColor3d('green_yellow'))
        ren3.SetViewport(0.51, 0.51, 1.0, 1.0)

        # config 4th render left-bottom
        #ren4.AddActor2D(actor4)
        ren4.SetBackground(colors.GetColor3d('DodgerBlue'))
        ren4.SetViewport(0.0, 0.0, 0.49, 0.49)

        renWin.AddRenderer(ren2)
        renWin.AddRenderer(ren3)
        renWin.AddRenderer(ren4) # 
        renWin.AddRenderer(ren) # eton bug--正交视图只能显示到第三个render中，与数量以及顺序都有关系，小于4个时候，最后显示正确，＞3个时候在哪里都不正确了


        #:help CTRL-V-alternative 
        #Set the interactor for the widgets
        iact = vtk.vtkRenderWindowInteractor()
        iact.SetRenderWindow(renWin)

        #style = vtk.vtkInteractorStyleTrackballCamera()  #vtkInteractorStyle()
        #iact.SetInteractorStyle(style)
        def add_text_label(iact, ren, info='3D', location=(0.0, 0.9)):
                # Create the TextActor
                text_actor = vtk.vtkTextActor()
                text_actor.SetInput(info)
                text_actor.GetTextProperty().SetColor(colors.GetColor3d('Lime'))

                # Create the text representation. Used for positioning the text_actor
                text_representation = vtk.vtkTextRepresentation()
                text_representation.ProportionalResizeOn()
                text_representation.GetPositionCoordinate().SetValue(*location)
                print("text_representation.GetProportionalResize=", text_representation.GetProportionalResize() )
                #text_representation.GetPosition2Coordinate().SetValue(1.0, 1.0)

                # Create the TextWidget
                # Note that the SelectableOff method MUST be invoked!
                # According to the documentation :
                #
                # SelectableOn/Off indicates whether the interior region of the widget can be
                # selected or not. If not, then events (such as left mouse down) allow the user
                # to 'move' the widget, and no selection is possible. Otherwise the
                # SelectRegion() method is invoked.
                text_widget = vtk.vtkTextWidget()
                text_widget.SetRepresentation(text_representation)

                text_widget.SetInteractor(iact)
                text_widget.SetTextActor(text_actor)
                text_widget.SelectableOff()
                text_widget.SetCurrentRenderer(ren)
                text_widget.On()
                return text_widget

        def add_text_label2(render):
                text = vtk.vtkTextActor()
                text.SetInput('T')
                tprop = text.GetTextProperty()
                tprop.SetFontFamilyToArial()
                tprop.ShadowOff()
                tprop.SetLineSpacing(1.0)
                tprop.SetFontSize(31)
                tprop.SetColor(colors.GetColor3d('antique_white'))
                text.SetDisplayPosition(280, 260)
                render.AddActor2D(text)
                return text
        
        text_widget = add_text_label(iact, ren)
        text_widget2 = add_text_label(iact, ren2, 'A', (0.9, 0.0))
        text_widget3 = add_text_label(iact, ren3, 'C', (0.0, 0.0))
        text_widget4 = add_text_label(iact, ren4, 'T', (0.9, 0.9))


        def enable_3d_view_imgPlaneWidges(widgets, ren, iact):
                for imgPlaneWidget in widgets:
                        imgPlaneWidget.SetCurrentRenderer(ren)
                        imgPlaneWidget.SetInteractor(iact)
                        imgPlaneWidget.On()

        enable_3d_view_imgPlaneWidges([planeWidgetX, planeWidgetY, planeWidgetZ], ren, iact)
        viewA=create_3_imgPlaneWidgets()
        enable_3d_view_imgPlaneWidges(viewA, ren2, iact)
        viewB=create_3_imgPlaneWidgets()
        enable_3d_view_imgPlaneWidges(viewB, ren3, iact)
        viewC=create_3_imgPlaneWidgets()
        enable_3d_view_imgPlaneWidges(viewC, ren4, iact)

        def create_3d_initial_view(render):
                # Create an initial interesting view
                render.ResetCamera()
                cam1 = render.GetActiveCamera()
                cam1.Elevation(110)
                cam1.SetViewUp(0, 0, -1)
                cam1.Azimuth(45)
                render.ResetCameraClippingRange()
        create_3d_initial_view(ren)

        def create_plane_initial_view(render, plane='x'):
                render.ResetCamera()
                cam1 = render.GetActiveCamera()
                print("viewup=",cam1.GetViewUp(), "GetViewAngle=", cam1.GetViewAngle(), "GetPosition=", cam1.GetPosition())
                cam1.Azimuth(90)
                print("viewup=",cam1.GetViewUp(), "GetViewAngle=", cam1.GetViewAngle(), "GetPosition=", cam1.GetPosition())
                #render.ResetCameraClippingRange()

        create_plane_initial_view(ren2)
        t3d = add_text_label2(ren4)

        iact.Initialize()
        #renWin.Render()
        iact.Start()
#============================================
if "__main__" == __name__:
    print("using pvtkPython:starting 3d view")
    main(sys.argv)
    print("process end~~~")
