//************************************************************************************************
//
// CCL Demo Application
//
// This file is part of Crystal Class Library (R)
// Copyright (c) 2025 CCL Software Licensing GmbH.
// All Rights Reserved.
//
// Licensed for use under either:
//  1. a Commercial License provided by CCL Software Licensing GmbH, or
//  2. GNU Affero General Public License v3.0 (AGPLv3).
// 
// You must choose and comply with one of the above licensing options.
// For more information, please visit ccl.dev.
//
// Filename    : graphicsdemo3d.cpp
// Description : 3D Graphics Demo
//
//************************************************************************************************

#define DEBUG_LOG 0

#include "../demoitem.h"
#include "exampletext.h"

#include "ccl/app/components/scenecomponent3d.h"
#include "ccl/app/controls/usersceneview3d.h"

#include "ccl/base/storage/url.h"

#include "ccl/public/gui/iparameter.h"
#include "ccl/public/gui/framework/itheme.h"
#include "ccl/public/gui/framework/iview3d.h"
#include "ccl/public/gui/framework/viewbox.h"
#include "ccl/public/gui/framework/idleclient.h"
#include "ccl/public/gui/framework/ianimation.h"
#include "ccl/public/gui/graphics/3d/iscene3d.h"
#include "ccl/public/gui/graphics/3d/itessellator3d.h"
#include "ccl/public/gui/graphics/3d/modelfactory3d.h"
#include "ccl/public/gui/graphics/3d/stockshader3d.h"
#include "ccl/public/gui/graphics/iimage.h"
#include "ccl/public/gui/graphics/igraphics.h"
#include "ccl/public/gui/graphics/ibitmap.h"
#include "ccl/public/gui/graphics/itextlayout.h"
#include "ccl/public/gui/graphics/graphicsfactory.h"
#include "ccl/public/gui/graphics/iuivalue.h"

#include "ccl/public/math/mathprimitives.h"

#include "ccl/public/plugservices.h"

namespace Tag 
{
	enum Graphics3DDemoTags
	{
		kCameraPosX = 1,
		kCameraPosY,
		kCameraPosZ,
		kCameraYawAngle,
		kCameraPitchAngle,
		kCameraRollAngle,
		kCameraFieldOfViewAngle,
		
		kGridNodeActive,
		kTeapotNodeActive,
		kOutlineCubeActive,
		kTransparentCubeActive,
		kSphereActive,
		kBillboardActive,
		kTextBillboardActive,
		kUserView3DActive,
		kAnimate
	};
}

using namespace CCL;

//************************************************************************************************
// DemoContent3D
//************************************************************************************************

class DemoContent3D: public Object,
                     public IGraphicsContent3D
{
public:
	// IGraphicsContent3D
	tresult CCL_API createContent (IGraphicsFactory3D& factory) override
	{
		AutoPtr<IGraphicsShader3D> vertexShader = factory.createShader (IGraphicsShader3D::kVertexShader, ResourceUrl ("shaders/vertexshader"));
		ASSERT (vertexShader)
		if(!vertexShader)
			return kResultFailed;

		AutoPtr<IGraphicsShader3D> pixelShader = factory.createShader (IGraphicsShader3D::kPixelShader, ResourceUrl ("shaders/pixelshader"));
		ASSERT (pixelShader)
		if(!pixelShader)
			return kResultFailed;

		AutoPtr<IVertexFormat3D> vertexFormat = factory.createVertexFormat (VertexP::kDescription, ARRAY_COUNT (VertexP::kDescription), vertexShader);
		ASSERT (vertexFormat)
		if(!vertexFormat)
			return kResultFailed;

		pipeline = factory.createPipeline ();
		ASSERT (pipeline)
		if(!pipeline)
			return kResultFailed;

		pipeline->setPrimitiveTopology (kPrimitiveTopologyTriangleList);
		pipeline->setVertexFormat (vertexFormat);
		pipeline->setVertexShader (vertexShader);
		pipeline->setPixelShader (pixelShader);

		vertexBuffer = factory.createBuffer (IGraphicsBuffer3D::kVertexBuffer, kBufferUsageDefault, sizeof(kVertices), sizeof(kVertices), kVertices);
		ASSERT (vertexBuffer)
		if(!vertexBuffer)
			return kResultFailed;

		return kResultOk;
	}

	tresult CCL_API releaseContent () override
	{
		vertexBuffer.release ();
		pipeline.release ();
		return kResultOk;
	}

	tresult CCL_API renderContent (IGraphics3D& graphics) override
	{
		graphics.setVertexBuffer (vertexBuffer, sizeof(VertexP));
		graphics.setPipeline (pipeline);

		graphics.draw (0, ARRAY_COUNT (kVertices));
		return kResultOk;
	}

	tresult CCL_API getContentProperty (Variant& value, ContentProperty3D propertyId) const override
	{
		switch(propertyId)
		{
		case kContentHint : value = kGraphicsContentTranslucent; return kResultOk;
		case kBackColor: value = Color (255, 0, 0, 50); return kResultOk;
		case kMultisampling: value = 2; return kResultOk;
		default : return kResultFailed;
		}
	}

	CLASS_INTERFACE (IGraphicsContent3D, Object)

private:
	AutoPtr<IGraphicsBuffer3D> vertexBuffer;
	AutoPtr<IGraphicsPipeline3D> pipeline;

	static const VertexP kVertices[3];
};

const VertexP DemoContent3D::kVertices[3] =
{
	{{ -0.5f, -0.5f, 0.0f }},
	{{  0.5f, -0.5f, 0.0f }},
	{{  0.0f,  0.5f, 0.0f }}
};

//************************************************************************************************
// DemoSceneView
//************************************************************************************************

class DemoSceneView: public UserSceneView3D
{
public:
	DECLARE_CLASS (DemoSceneView, UserSceneView3D)

	DemoSceneView (RectRef size = Rect (), StyleRef style = 0, StringRef title = 0);

	// UserSceneView3D
	bool onMouseEnter (const MouseEvent& event) override;
	bool onMouseMove (const MouseEvent& event) override;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_CLASS_HIDDEN (DemoSceneView, UserSceneView3D)

//////////////////////////////////////////////////////////////////////////////////////////////////

DemoSceneView::DemoSceneView (RectRef size, StyleRef style, StringRef title)
: UserSceneView3D (size, style, title)
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool DemoSceneView::onMouseEnter (const MouseEvent& event)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool DemoSceneView::onMouseMove (const MouseEvent& event)
{
	IScene3D* scene = sceneView->getSceneRenderer ().getIScene ();
	if(scene == nullptr)
		return true;

	ISceneNode3D* billboard = scene->getChildren ()->findNode ("textBillboard");
	UnknownPtr<IModelNode3D> modelNode (billboard);
	UnknownPtr<IModel3D> model = modelNode ? modelNode->getModelData () : nullptr;
	UnknownPtr<ITextureMaterial3D> material = model ? model->getFirstMaterial () : nullptr;

	if(!material.isValid ())
		return true;

	SceneEdit3D scope (scene, billboard, IScene3D::kUserEdit);
	if(findNodeAt (event.where) == billboard)
		material->setOpacity (.9f);
	else
		material->setOpacity (.6f);

	return true;
}

//************************************************************************************************
// DemoSceneComponent
//************************************************************************************************

class DemoSceneComponent: public SceneComponent3D,
						  public IdleClient
{
public:
	DECLARE_CLASS (DemoSceneComponent, SceneComponent3D)

	DemoSceneComponent ();

	// SceneComponent3D
	tbool CCL_API paramChanged (IParameter* param) override;
	void CCL_API rendererAttached (ISceneRenderer3D& sceneRenderer) override;
	void CCL_API rendererDetached (ISceneRenderer3D& sceneRenderer) override;
	void CCL_API sceneChanged (IScene3D& scene, ISceneNode3D* node, int editFlags) override;

	// IdleClient
	void onIdleTimer () override;

	CLASS_INTERFACE2 (ISceneHandler3D, ITimerTask, SceneComponent3D)

private:
	static constexpr CoordF kCameraPosRange = 20;
	static constexpr float kFieldOfViewAngleMin = 0.1f;
	static constexpr float kFieldOfViewAngleMax = 135.f;

	SharedPtr<ICamera3D> camera;
	AutoPtr<ITextLayout> textLayout;
	AutoPtr<IImage> dynamicTexture;
	AutoPtr<ITextureMaterial3D> dynamicTextureMaterial;
	ILightSource3D* ambientLight;
	ILightSource3D* directionalLight;
	UnknownPtr<IPointLight3D> bluePointLight;
	UnknownPtr<IPointLight3D> redPointLight;
	int idleCounter;
	
	ICamera3D* addCamera (IScene3D* scene);
	void addLight (IScene3D* scene);
	void updateGrid (IScene3D* scene, bool visible);
	void updateTeapot (IScene3D* scene, bool visible);
	void updateOutlineCube (IScene3D* scene, bool visible);
	void updateTransparentCube (IScene3D* scene, bool visible);
	void updateSphere (IScene3D* scene, bool visible);
	void updateBillboard (IScene3D* scene, bool visible);
	void updateTextBillboard (IScene3D* scene, bool visible);
	void createCameraParameters ();
	void createNodeParameters ();
	void applyToCamera ();
	void syncCameraParams ();
	void updateDynamicTextBillboard ();
	void setNodeNameFromTag (IModelNode3D* node, int tag);
	ISceneNode3D* findNodeByTag (int tag);
	bool removeNodeWithTag (int tag);
	void animateScene ();

	// SceneComponent3D
	void buildScene () override;
	UserSceneView3D* createSceneView (RectRef bounds) override;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

DEFINE_CLASS_HIDDEN (DemoSceneComponent, SceneComponent3D)

//////////////////////////////////////////////////////////////////////////////////////////////////

DemoSceneComponent::DemoSceneComponent ()
: SceneComponent3D ("DemoSceneComponent"),
  ambientLight (nullptr),
  directionalLight (nullptr),
  bluePointLight (nullptr),
  redPointLight (nullptr),
  idleCounter (0)
{
	createCameraParameters ();
	buildScene ();
	createNodeParameters ();

	setMainCamera (camera);

	paramList.addParam ("animate", Tag::kAnimate);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::buildScene ()
{
	// build 3D scene
	scene = ccl_new<IScene3D> (ClassID::Scene3D);
	scene->setHandler (this);

	SceneEdit3D scope (scene);
	camera = addCamera (scene);
	addLight (scene);

	SuperClass::buildScene ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

UserSceneView3D* DemoSceneComponent::createSceneView (RectRef bounds)
{
	return NEW DemoSceneView (bounds);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CCL_API DemoSceneComponent::rendererAttached (ISceneRenderer3D& sceneRenderer)
{
	CCL_PRINTF ("Scene renderer attached %p\n", &sceneRenderer)
	startTimer (1, true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CCL_API DemoSceneComponent::rendererDetached (ISceneRenderer3D& sceneRenderer)
{
	CCL_PRINTF ("Scene renderer detached %p\n", &sceneRenderer)
	stopTimer ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CCL_API DemoSceneComponent::sceneChanged (IScene3D& scene, ISceneNode3D* node, int editFlags)
{
	if(node == camera || node == nullptr)
		syncCameraParams ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::onIdleTimer ()
{
	updateDynamicTextBillboard ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

tbool CCL_API DemoSceneComponent::paramChanged (IParameter* param)
{
	SceneEdit3D scope (scene);

	switch(param->getTag ())
	{
	case Tag::kCameraPosX :
	case Tag::kCameraPosY :
	case Tag::kCameraPosZ :
	case Tag::kCameraYawAngle :
	case Tag::kCameraPitchAngle :
	case Tag::kCameraRollAngle :
	case Tag::kCameraFieldOfViewAngle :
		applyToCamera ();
		break;

	case Tag::kGridNodeActive : updateGrid (scene, param->getValue ()); break;
	case Tag::kTeapotNodeActive : updateTeapot (scene, param->getValue ()); break;
	case Tag::kOutlineCubeActive : updateOutlineCube (scene, param->getValue ()); break;
	case Tag::kTransparentCubeActive : updateTransparentCube (scene, param->getValue ()); break;
	case Tag::kSphereActive : updateSphere (scene, param->getValue ()); break;
	case Tag::kBillboardActive : updateBillboard (scene, param->getValue ()); break;
	case Tag::kTextBillboardActive : updateTextBillboard (scene, param->getValue ()); break;

	case Tag::kAnimate :
		animateScene ();
		break;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

ICamera3D* DemoSceneComponent::addCamera (IScene3D* scene)
{
	auto cam = ccl_new<ICamera3D> (ClassID::Camera3D);
	cam->setNodeName ("cam1");

	constexpr float kDefaultFieldOfViewAngle = 33;
	const PointF3D kDefaultCameraPos (6.f, 8.f, 12.f);

	cam->setPosition (kDefaultCameraPos);
	cam->lookAt (PointF3D (0, 0, 0));
	cam->setFieldOfViewAngle (kDefaultFieldOfViewAngle);

	paramList.byTag (Tag::kCameraPosX)->setDefaultValue (kDefaultCameraPos.x);
	paramList.byTag (Tag::kCameraPosY)->setDefaultValue (kDefaultCameraPos.y);
	paramList.byTag (Tag::kCameraPosZ)->setDefaultValue (kDefaultCameraPos.z);

	paramList.byTag (Tag::kCameraYawAngle)->setDefaultValue (cam->getYawAngle ());
	paramList.byTag (Tag::kCameraPitchAngle)->setDefaultValue (cam->getPitchAngle ());
	paramList.byTag (Tag::kCameraRollAngle)->setDefaultValue (cam->getRollAngle ());

	paramList.byTag (Tag::kCameraFieldOfViewAngle)->setDefaultValue (kDefaultFieldOfViewAngle);

	cam->getConstraints ()->addConstraints (NEW TranslationConstraints3D ({-kCameraPosRange, -kCameraPosRange, -kCameraPosRange}, {kCameraPosRange, kCameraPosRange, kCameraPosRange}));

	scene->getChildren ()->addNode (cam);
	return cam;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::addLight (IScene3D* scene)
{
	ambientLight = ccl_new<ILightSource3D> (ClassID::AmbientLight3D);
	ambientLight->setLightColor (ColorF (0.22f, 0.24f, 0.30f));
	scene->getChildren ()->addNode (ambientLight);

	directionalLight = ccl_new<ILightSource3D> (ClassID::DirectionalLight3D);
	directionalLight->setRollAngle (0.1f);
	directionalLight->setPitchAngle (-0.9f);
	directionalLight->setYawAngle (-0.2f);
	directionalLight->setLightColor (ColorF (0.25f, 0.22f, 0.22f));
	scene->getChildren ()->addNode (directionalLight);

	bluePointLight = ccl_new<ILightSource3D> (ClassID::PointLight3D);
	bluePointLight->setPosition ({2.5f, 1.f, 1.f});
	bluePointLight->setLightColor (Color (0x8E, 0xDB, 0xFF));
	bluePointLight->setAttenuationRadius (3.f);
	bluePointLight->setAttenuationMinimum (0.5f);
	scene->getChildren ()->addNode (bluePointLight);

	redPointLight = ccl_new<ILightSource3D> (ClassID::PointLight3D);
	redPointLight->setPosition ({0.f, 1.f, 2.5f});
	redPointLight->setLightColor (Color (0xFF, 0xD0, 0xC6));
	scene->getChildren ()->addNode (redPointLight);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::updateGrid (IScene3D* scene, bool visible)
{
	if(!visible)
		if(removeNodeWithTag (Tag::kGridNodeActive))
			return;

	auto* gridModelNode = ccl_new<IModelNode3D> (ClassID::ModelNode3D);
	#if 1
	AutoPtr<IMaterial3D> gridMaterial = ModelFactory3D::createSolidColorMaterial (Colors::kLtGray);
	#else
	AutoPtr<IMaterial3D> gridMaterial = ModelFactory3D::createTextureMaterial (demoTexture, Colors::kLtGray);
	#endif
	gridMaterial->setDepthBias (20.f);
	gridModelNode->setModelData (ModelFactory3D::createGrid (4, 4, 1.f, 1.f, gridMaterial));

	gridModelNode->setPosition ({-2.5f, 0.f, -2.5f});
	setNodeNameFromTag (gridModelNode, Tag::kGridNodeActive);
	scene->getChildren ()->addNode (gridModelNode);
};

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::updateTeapot (IScene3D* scene, bool visible)
{
	if(!visible)
		if(removeNodeWithTag (Tag::kTeapotNodeActive))
			return;

	UnknownPtr<IModel3D> demoModel = getTheme ()->getResource ("DemoModel");
	ASSERT (demoModel.isValid ())

	auto* teapotModelNode = ccl_new<IModelNode3D> (ClassID::ModelNode3D);
	AutoPtr<ISolidColorMaterial3D> demoModelMaterial = demoModel->createSolidColorMaterial ();
	demoModelMaterial->setMaterialColor (Colors::kGray);
	demoModelMaterial->setShininess (80.f);
	demoModel->setMaterialForGeometries (demoModelMaterial);

	teapotModelNode->setModelData (demoModel);
	teapotModelNode->setPosition ({0.f, 1.f, 0.f});
	setNodeNameFromTag (teapotModelNode, Tag::kTeapotNodeActive);
	scene->getChildren ()->addNode (teapotModelNode);
};

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::updateOutlineCube (IScene3D* scene, bool visible)
{
	if(!visible)
		if(removeNodeWithTag (Tag::kOutlineCubeActive))
			return;

	auto* outlineCubeModelNode = ccl_new<IModelNode3D> (ClassID::ModelNode3D);
	AutoPtr<IModel3D> cubeModel = ccl_new<IModel3D> (ClassID::Model3D);

	AutoPtr<ICubeTessellator3D> cubeTessellator = ccl_new<ICubeTessellator3D> (ClassID::CubeTessellator3D);
	cubeTessellator->generate (ITessellator3D::kGenerateTextureCoordinates | ITessellator3D::kGenerateInverseNormals | ITessellator3D::kWindingOrderCW);
	IGeometry3D* innerCubeGeometry = cubeModel->createGeometry ();
	innerCubeGeometry->copyFrom (*cubeTessellator);
	cubeTessellator->generate (ITessellator3D::kGenerateTextureCoordinates | ITessellator3D::kWindingOrderCCW);
	IGeometry3D* outerCubeGeometry = cubeModel->createGeometry ();
	outerCubeGeometry->copyFrom (*cubeTessellator);

	cubeModel->addGeometry (outerCubeGeometry);
	cubeModel->addGeometry (innerCubeGeometry);

	UnknownPtr<IBitmap> outlineTexture = getTheme ()->getImage ("OutlineTexture");
	cubeModel->setMaterialForGeometries (ModelFactory3D::createTextureMaterial (outlineTexture));

	outlineCubeModelNode->setModelData (cubeModel);
	outlineCubeModelNode->setPosition ({2.5f, 1.f, -1.f});
	outlineCubeModelNode->setScaleX (.5f);
	outlineCubeModelNode->setScaleY (.5f);
	outlineCubeModelNode->setScaleZ (.5f);
	outlineCubeModelNode->setRollAngle (2.5f);
	outlineCubeModelNode->setYawAngle (2.f);
	setNodeNameFromTag (outlineCubeModelNode, Tag::kOutlineCubeActive);
	scene->getChildren ()->addNode (outlineCubeModelNode);
};

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::updateTransparentCube (IScene3D* scene, bool visible)
{
	if(!visible)
		if(removeNodeWithTag (Tag::kTransparentCubeActive))
			return;

	UnknownPtr<IBitmap> demoTexture = getTheme ()->getImage ("DemoTexture");
	auto* texturedCubeModelNode = ccl_new<IModelNode3D> (ClassID::ModelNode3D);
	AutoPtr<IModel3D> texturedCubeModel = ccl_new<IModel3D> (ClassID::Model3D);

	AutoPtr<ICubeTessellator3D> cubeTessellator = ccl_new<ICubeTessellator3D> (ClassID::CubeTessellator3D);
	cubeTessellator->generate (ITessellator3D::kGenerateTextureCoordinates | ITessellator3D::kWindingOrderCCW);
	IGeometry3D* outerCubeGeometry = texturedCubeModel->createGeometry ();
	outerCubeGeometry->copyFrom (*cubeTessellator);

	texturedCubeModel->addGeometry (outerCubeGeometry);
	AutoPtr<ITextureMaterial3D> textureMaterial = ModelFactory3D::createTextureMaterial (demoTexture, Colors::kBlue);
	textureMaterial->setOpacity (.7f);
	textureMaterial->setLightMask (ambientLight->getLightMask () | directionalLight->getLightMask () | redPointLight->getLightMask ());
	texturedCubeModel->setMaterialForGeometries (textureMaterial);
	texturedCubeModelNode->setModelData (texturedCubeModel);
	texturedCubeModelNode->setPosition ({4.f, 1.f, 1.f});
	texturedCubeModelNode->setScaleX (1.2f);
	texturedCubeModelNode->setScaleY (1.2f);
	texturedCubeModelNode->setScaleZ (1.2f);
	setNodeNameFromTag (texturedCubeModelNode, Tag::kTransparentCubeActive);
	scene->getChildren ()->addNode (texturedCubeModelNode);
};

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::updateSphere (IScene3D* scene, bool visible)
{
	if(!visible)
		if(removeNodeWithTag (Tag::kSphereActive))
			return;

	auto* sphereModelNode = ccl_new<IModelNode3D> (ClassID::ModelNode3D);
	#if 1
	AutoPtr<IMaterial3D> sphereMaterial = ModelFactory3D::createSolidColorMaterial ({255, 0, 0, 200});
	#else
	UnknownPtr<IBitmap> demoTexture = getTheme ()->getImage ("DemoTexture");
	AutoPtr<IMaterial3D> sphereMaterial = ModelFactory3D::createTextureMaterial (demoTexture, {255, 0, 0, 200});
	#endif
	sphereMaterial->setDepthBias (-1.0f);
	sphereMaterial->setLightMask (ambientLight->getLightMask () | directionalLight->getLightMask () | bluePointLight->getLightMask ());
	sphereModelNode->setModelData (ModelFactory3D::createSphere (0.5f, 50, 50, sphereMaterial));
	sphereModelNode->setPosition ({0.f, 1.f, 2.5f});
	setNodeNameFromTag (sphereModelNode, Tag::kSphereActive);
	scene->getChildren ()->addNode (sphereModelNode);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::updateBillboard (IScene3D* scene, bool visible)
{
	if(!visible)
		if(removeNodeWithTag (Tag::kBillboardActive))
			return;

	UnknownPtr<IBitmap> demoTexture = getTheme ()->getImage ("DemoTexture");
	auto* billboardModelNode = ccl_new<IModelNode3D> (ClassID::ModelNode3D);
	billboardModelNode->setModelData (ModelFactory3D::createBillboard (ModelFactory3D::createTextureMaterial (demoTexture)));
	billboardModelNode->setPosition ({0.f, 1.f, 2.5f});
	billboardModelNode->setScaleX (0.8f * demoTexture->getPixelSize ().x / demoTexture->getPixelSize ().y);
	billboardModelNode->setScaleY (0.8f);
	setNodeNameFromTag (billboardModelNode, Tag::kBillboardActive);
	scene->getChildren ()->addNode (billboardModelNode);
};

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::updateTextBillboard (IScene3D* scene, bool visible)
{
	if(!visible)
		if(removeNodeWithTag (Tag::kTextBillboardActive))
			return;

	textLayout = GraphicsFactory::createTextLayout ();
	Font font (getStandardFont ());
	font.setSize (40);
	String text (kExampleText);	
	textLayout->construct (text, 512, 512, font, ITextLayout::kMultiLine, TextFormat (Alignment::kCenter, TextFormat::kWordBreak));
	dynamicTexture = GraphicsFactory::createBitmap (512, 512, IBitmap::kRGBAlpha);
	dynamicTextureMaterial = ModelFactory3D::createTextureMaterial (UnknownPtr<IBitmap> (dynamicTexture), Colors::kWhite);
	dynamicTextureMaterial->setLightMask (0);
	dynamicTextureMaterial->setOpacity (.6f);
	
	auto* dynamicTextureModelNode = ccl_new<IModelNode3D> (ClassID::ModelNode3D);
	dynamicTextureModelNode->setModelData (ModelFactory3D::createBillboard (dynamicTextureMaterial));
	dynamicTextureModelNode->setPosition ({2.5f, 1.f, 0.f});
	dynamicTextureModelNode->enableHitTesting (true);
	setNodeNameFromTag (dynamicTextureModelNode, Tag::kTextBillboardActive);
	scene->getChildren ()->addNode (dynamicTextureModelNode);
};

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::createCameraParameters ()
{
	auto addFloatParam = [&] (double min, double max, StringID name, int tag, float defaultValue = 0.f)
	{
		IParameter* param = paramList.addFloat (min, max, name, tag);
		param->setValue (defaultValue);
		param->setDefaultValue (defaultValue);
	};

	addFloatParam (-kCameraPosRange, kCameraPosRange, "cameraPosX", Tag::kCameraPosX, 0);
	addFloatParam (-kCameraPosRange, kCameraPosRange, "cameraPosY", Tag::kCameraPosY, 0);
	addFloatParam (-kCameraPosRange, kCameraPosRange, "cameraPosZ", Tag::kCameraPosZ, 0);

	addFloatParam (-180, 180, "cameraYawAngle", Tag::kCameraYawAngle, 0);
	addFloatParam (-180, 180, "cameraPitchAngle", Tag::kCameraPitchAngle, 0);
	addFloatParam (-180, 180, "cameraRollAngle", Tag::kCameraRollAngle, 0);

	addFloatParam (kFieldOfViewAngleMin, kFieldOfViewAngleMax, "cameraFieldOfViewAngle", Tag::kCameraFieldOfViewAngle, kFieldOfViewAngleMin);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::createNodeParameters ()
{
	paramList.addParam ("grid", Tag::kGridNodeActive)->setValue (false, true);
	paramList.addParam ("teapot", Tag::kTeapotNodeActive)->setValue (true, true);
	paramList.addParam ("outlineCube", Tag::kOutlineCubeActive)->setValue (false, true);
	paramList.addParam ("transparentCube", Tag::kTransparentCubeActive)->setValue (false, true);
	paramList.addParam ("sphere", Tag::kSphereActive)->setValue (false, true);
	paramList.addParam ("billboard", Tag::kBillboardActive)->setValue (false, true);
	paramList.addParam ("textBillboard", Tag::kTextBillboardActive)->setValue (false, true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::applyToCamera ()
{
	if(camera.isValid () == false)
		return;

	PointF3D position;
	position.x = paramList.byTag (Tag::kCameraPosX)->getValue ();
	position.y = paramList.byTag (Tag::kCameraPosY)->getValue ();
	position.z = paramList.byTag (Tag::kCameraPosZ)->getValue ();
	camera->setPosition (position);

	camera->setYawAngle (Math::degreesToRad (paramList.byTag (Tag::kCameraYawAngle)->getValue ().asFloat ()));
	camera->setPitchAngle (Math::degreesToRad (paramList.byTag (Tag::kCameraPitchAngle)->getValue ().asFloat ()));
	camera->setRollAngle (Math::degreesToRad (paramList.byTag (Tag::kCameraRollAngle)->getValue ().asFloat ()));

	float fieldOfViewAngle = paramList.byTag (Tag::kCameraFieldOfViewAngle)->getValue ().asFloat ();
	camera->setFieldOfViewAngle (fieldOfViewAngle);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::syncCameraParams ()
{
	if(camera.isValid () == false)
		return;

	PointF3DRef position = camera->getPosition ();
	paramList.byTag (Tag::kCameraPosX)->setValue (position.x);
	paramList.byTag (Tag::kCameraPosY)->setValue (position.y);
	paramList.byTag (Tag::kCameraPosZ)->setValue (position.z);

	paramList.byTag (Tag::kCameraYawAngle)->setValue (Math::radToDegrees (camera->getYawAngle ()));
	paramList.byTag (Tag::kCameraPitchAngle)->setValue (Math::radToDegrees (camera->getPitchAngle ()));
	paramList.byTag (Tag::kCameraRollAngle)->setValue (Math::radToDegrees (camera->getRollAngle ()));

	paramList.byTag (Tag::kCameraFieldOfViewAngle)->setValue (camera->getFieldOfViewAngle ());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::updateDynamicTextBillboard ()
{
	if(ISceneNode3D* billboard = scene->getChildren ()->findNode ("textBillboard"))
	{
		Rect size (0, 0, dynamicTexture->getWidth (), dynamicTexture->getHeight ());
		if(AutoPtr<IGraphics> graphics = GraphicsFactory::createBitmapGraphics (dynamicTexture))
		{
			graphics->clearRect (size);

			Point offset (0, size.getHeight () - idleCounter);
			graphics->drawTextLayout (offset, textLayout, SolidBrush (Colors::kBlack));
		}

		idleCounter++;
		if(idleCounter > 2 * size.getHeight ())
			idleCounter = 0;

		SceneEdit3D scope (scene, billboard);
		dynamicTextureMaterial->setTextureFlags (kDiffuseTexture, kTextureMipmapEnabled);
		dynamicTextureMaterial->setTexture (kDiffuseTexture, UnknownPtr<IBitmap> (dynamicTexture));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::setNodeNameFromTag (IModelNode3D* node, int tag)
{
	if(IParameter* param = paramList.byTag (tag))
		node->setNodeName (param->getName ());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

ISceneNode3D* DemoSceneComponent::findNodeByTag (int tag)
{
	if(IParameter* param = paramList.byTag (tag))
		return scene->getChildren ()->findNode (param->getName ());
	else
		return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool DemoSceneComponent::removeNodeWithTag (int tag)
{
	if(AutoPtr<ISceneNode3D> node = findNodeByTag (tag))
		if(scene->getChildren ()->removeNode (node) == kResultOk)
			return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DemoSceneComponent::animateScene ()
{
	ISceneNode3D* node = findNodeByTag (Tag::kTeapotNodeActive);
	if(!node)
		return;

	AutoPtr<IBasicAnimation> animation = ccl_new<IBasicAnimation> (ClassID::BasicAnimation);
	AnimationDescription description;
	description.duration = 1.5;
	description.timingType = kTimingEaseInOut;
	description.resetMode = IAnimation::kResetForwards;
	animation->setDescription (description);

	AutoPtr<IUIValue> startValue = GraphicsFactory::createValue ();
	startValue->fromPointF3D ({0.f, 1.f, 0.f});
	AutoPtr<IUIValue> endValue = GraphicsFactory::createValue ();
	endValue->fromPointF3D ({0.f, 2.f, 0.f});
	animation->setStartValue (startValue);
	animation->setEndValue (endValue);

	node->addAnimation (ISceneNode3D::kPosition, animation);
}

//************************************************************************************************
// Graphics3DDemo
//************************************************************************************************

class Graphics3DDemo: public DemoComponent
{
public:
	Graphics3DDemo ();
	
	// Component
	IView* CCL_API createView (StringID name, VariantRef data, const Rect& bounds) override;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

Graphics3DDemo::Graphics3DDemo ()
{
	addChild (NEW DemoSceneComponent);

	paramList.addParam ("user3D", Tag::kUserView3DActive);
	paramList.addParam ("animate", Tag::kAnimate);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

IView* CCL_API Graphics3DDemo::createView (StringID name, VariantRef data, const Rect& bounds)
{
	if(name == "UserView3D")
	{
		ViewBox view3d (ClassID::UserView3D, bounds);
		UnknownPtr<IView3D> (view3d)->set3DContent (AutoPtr<IGraphicsContent3D> (NEW DemoContent3D));
		return view3d;
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

REGISTER_DEMO ("Graphics", "Graphics 3D", Graphics3DDemo)
