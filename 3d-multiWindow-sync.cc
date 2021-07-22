//from: https://blog.csdn.net/calmreason/article/details/86499392
#include "vtkConeSource.h"
#include "vtkCubeSource.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkCamera.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkSmartPointer.h"
#include "vtkInteractorStyleSwitch.h"
 
static vtkSmartPointer<vtkRenderer> renderer01;
static vtkSmartPointer<vtkRenderer> renderer02;
static vtkSmartPointer<vtkRenderWindow> renderWindow01;
static vtkSmartPointer<vtkRenderWindow> renderWindow02;
 
class vtkMyCameraCallback : public vtkCommand
{
public:
	static vtkMyCameraCallback *New()
	{
		return new vtkMyCameraCallback;
	}
	virtual void Execute(vtkObject *caller, unsigned long, void*)
 
	{
		vtkCamera *camera = static_cast<vtkCamera*>(caller);
		double camPos[3];
		double focalPos[3];
		double upVector[3];
		camera->GetPosition(camPos);
		camera->GetFocalPoint(focalPos);
		camera->GetViewUp(upVector);
		renderer02->SetActiveCamera(camera);
		renderWindow02->Render();
	}
};
 
int main(int argc, char *argv[])
{
	vtkSmartPointer<vtkConeSource> cone =	vtkSmartPointer<vtkConeSource>::New();
	vtkSmartPointer<vtkCubeSource> cube = vtkSmartPointer<vtkCubeSource>::New();
	cone->SetResolution(20);
	cone->Update();
	cube->Update();
	vtkSmartPointer<vtkPolyDataMapper> coneMapper =	vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkPolyDataMapper> cubeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	coneMapper->SetInputData(cone->GetOutput());
	cubeMapper->SetInputData(cube->GetOutput());
	coneMapper->Update();
	cubeMapper->Update();
	vtkSmartPointer<vtkActor> coneActor =	vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> cubeActor = vtkSmartPointer<vtkActor>::New();
	coneActor->SetMapper(coneMapper);
	cubeActor->SetMapper(cubeMapper);
 
	renderer01 = vtkSmartPointer<vtkRenderer>::New();
	renderer02 = vtkSmartPointer<vtkRenderer>::New();
	renderer01->AddActor(coneActor);
	renderer02->AddActor(cubeActor);
	// RENDER-WINDOW
 
	renderWindow01 = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow02 = vtkSmartPointer<vtkRenderWindow>::New();
 
	renderWindow01->AddRenderer(renderer01);
	renderWindow02->AddRenderer(renderer02);
 
	vtkSmartPointer<vtkRenderWindowInteractor> windowInteractor01 = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkInteractorStyleSwitch> trackballStyle =	vtkSmartPointer<vtkInteractorStyleSwitch>::New();
	trackballStyle->SetCurrentStyleToTrackballCamera();
	windowInteractor01->SetInteractorStyle(trackballStyle);
	windowInteractor01->SetRenderWindow(renderWindow01);
 
	// User event handling
	// Catch camera event   
	vtkSmartPointer<vtkMyCameraCallback> cameraCallback =vtkSmartPointer<vtkMyCameraCallback>::New();
	renderer01->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent,cameraCallback);
	renderer01->ResetCamera();
	windowInteractor01->Initialize();
	windowInteractor01->Start();
	return 0;
}
