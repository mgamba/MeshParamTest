#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"
#include "cinder/gl/gl.h"

#include "cinder/CameraUi.h"
#include "cinder/GeomIo.h"
#include "cinder/ImageIo.h"
#include "cinder/Rand.h"
#include "SimplexNoise.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void prepareSettings( App::Settings *settings )
{
    settings->setHighDensityDisplayEnabled();
    settings->setWindowSize( 720, 720 );
    settings->setMultiTouchEnabled( false );
}

enum HeightFunction { sine, uniform, randnoise, fractal, noise };
const vector<string> heightFunctionNames = { "sine", "uniform", "randnoise", "fractal", "noise" };

class MeshParamTestApp : public App {
public:
    void setup();
    void resize();
    void update();
    void draw();
    
private:
    CameraPersp                mCamera;
    params::InterfaceGlRef    mParams;
    float                    mObjSize;
    quat                    mObjOrientation;
    
    void                    setupParams();
    gl::VboMeshRef          mVboMesh;
    gl::TextureRef          mTexture;
    CameraUi                mCamUi;
    void                    setupPlane();
    void                    udpatePlaneHeights();
    void                    setupShader();
    gl::GlslProgRef         mGlsl;
    void                    updateNoise();
    SimplexNoise            mNoise;
    float                   mNoiseFrequency;
    float                   mNoiseAmplitude;
    float                   mNoiseLacunarity;
    float                   mNoisePersistence;
    int                     mOctaves;
    void                    updatePlaneDimensions();
    int                     mPlaneSize;
    int                     mPlaneSubdivisions;
    void                    setPlaneSize(int size);
    void                    setPlaneSubdivisions(int subdivisions);
    int                     getPlaneSize() { return mPlaneSize; }
    int                     getPlaneSubdivisions() { return mPlaneSubdivisions; }
    HeightFunction          mHeightFunction;
    int                     mSelectedHeightFunction;
    vec3                    mCameraEyePoint;
    vec3                    mCameraTarget;
    float                   mHeightMult;
};

void MeshParamTestApp::setPlaneSubdivisions( int subdivisions)
{
    mPlaneSubdivisions = subdivisions;
    updatePlaneDimensions();
}

void MeshParamTestApp::setPlaneSize( int size)
{
    mPlaneSize = size;
    updatePlaneDimensions();
}

void MeshParamTestApp::setup()
{
    mHeightFunction = fractal;
    mSelectedHeightFunction = fractal;
    setupParams();
    setupShader();
    setupPlane();
    udpatePlaneHeights();
}

void MeshParamTestApp::updateNoise()
{
    // successive octaves of coherent noise, each with higher frequency and lower amplitude
    cout << "updating noise " << mNoiseFrequency << ", " << mNoiseAmplitude << ", " << mNoiseLacunarity << ", " << mNoisePersistence << "\n";
    mNoise.mFrequency = mNoiseFrequency;
    mNoise.mAmplitude = mNoiseAmplitude;
    mNoise.mLacunarity = mNoiseLacunarity;
    mNoise.mPersistence = mNoisePersistence;
}

void MeshParamTestApp::setupShader()
{
    // setup shader
//    try {
//        mGlsl = gl::GlslProg::create( gl::GlslProg::Format().vertex( loadAsset( "basic.vert" ) )
//                                     .fragment( loadAsset( "basic.frag" ) )
//                                     .geometry( loadAsset( "basic.geom" ) ) );
//    }
//    catch( gl::GlslProgCompileExc ex ) {
//        cout << ex.what() << endl;
//        quit();
//    }
}

void MeshParamTestApp::setupPlane()
{
    updatePlaneDimensions();
    
    mTexture = gl::Texture::create(loadImage(loadAsset("cinder_logo.png")),
                                   gl::Texture::Format().loadTopDown() );
    
    mCamUi = CameraUi( &mCamera, getWindow() );
}

void MeshParamTestApp::updatePlaneDimensions()
{
    // create some geometry using a geom::Plane
    auto plane = geom::Plane().size( vec2(mPlaneSize, mPlaneSize) ).subdivisions( ivec2(mPlaneSubdivisions, mPlaneSubdivisions) );
    
    // Specify two planar buffers - positions are dynamic because they will be modified
    // in the update() loop. Tex Coords are static since we don't need to update them.
    vector<gl::VboMesh::Layout> bufferLayout = {
        gl::VboMesh::Layout().usage( GL_DYNAMIC_DRAW ).attrib( geom::Attrib::POSITION, 3 ),
        gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::TEX_COORD_0, 2 )
    };
    
    mVboMesh = gl::VboMesh::create( plane, bufferLayout );
}

void MeshParamTestApp::udpatePlaneHeights()
{
    float offset = getElapsedSeconds() * 4.0f;
    
    // Dynmaically generate our new positions based on a sin(x) + cos(z) wave
    // We set 'orphanExisting' to false so that we can also read from the position buffer, though keep
    // in mind that this isn't the most efficient way to do cpu-side updates. Consider using VboMesh::bufferAttrib() as well.
    
    auto mappedPosAttrib = mVboMesh->mapAttrib3f( geom::Attrib::POSITION, false );
    for( int i = 0; i < mVboMesh->getNumVertices(); i++ ) {
        vec3 &pos = *mappedPosAttrib;
        switch (mHeightFunction) {
            case sine:
                mappedPosAttrib->y = mHeightMult * sinf( pos.x * 1.1467f + offset ) * 0.323f + cosf( pos.z * 0.7325f + offset ) * 0.431f;
                break;
            case uniform:
                mappedPosAttrib->y = 1;
                break;
            case randnoise:
                mappedPosAttrib->y = Rand::randFloat(1);
                break;
            case fractal:
                mappedPosAttrib->y = mHeightMult * mNoise.fractal(mOctaves, pos.x, pos.z);
                break;
            case noise:
                mappedPosAttrib->y = mHeightMult * mNoise.noise(pos.x, pos.z);
                break;
            default:
                break;
        }
        
        ++mappedPosAttrib;
    }
    mappedPosAttrib.unmap();
}

void MeshParamTestApp::setupParams()
{
    // camera params
    mCameraEyePoint = vec3( 20.41, 30.3, 30.93 );
    mCameraTarget = vec3( 2.41, 3.81, 5.68 );
    
    // cube params
    mObjSize = 4;
    
    // mesh params
    mPlaneSize = 20;
    mPlaneSubdivisions = 50;
    mHeightMult = 1.0;
    
    // fractal params
    mOctaves = 2;
    
    // noise params
    mNoiseFrequency = 1.0f; // Frequency of an octave of noise is the "width" of the pattern
    mNoiseAmplitude = 1.0f; // Amplitude of an octave of noise it the "height" of its feature
    mNoiseLacunarity = 2.0f; // Lacunarity specifies frequency multipler between successive octaves (typically 2.0)
    mNoisePersistence = 0.5f; // Persistence is loss of amplitude between successive octabes (usually 1/lacunarity)
    
    // Create the interface and give it a name.
    mParams = params::InterfaceGl::create( getWindow(), "App parameters", toPixels( ivec2( 200, 400 ) ) );
    
    mParams->addParam( "Rotation", &mObjOrientation ).group("Camera Params");
    mParams->addParam( "Camera EyePoint", &mCameraEyePoint ).group("Camera Params");
    mParams->addParam( "Camera Target", &mCameraTarget ).group("Camera Params");
    
    // Setup some basic parameters.
    mParams->addParam( "Cube Size", &mObjSize ).min( 0.1f ).max( 20.5f ).keyIncr( "z" ).keyDecr( "Z" ).precision( 2 ).step( 0.02f );
    
    mParams->addSeparator();
    
    mParams->addParam( "Height Function", heightFunctionNames, &mSelectedHeightFunction )
    .updateFn( [this] { mHeightFunction = HeightFunction(mSelectedHeightFunction); } );
    mParams->addParam( "Height Function", heightFunctionNames, &mSelectedHeightFunction )
    .updateFn( [this] { mHeightFunction = HeightFunction(mSelectedHeightFunction); } );
    
    mParams->addParam("Octaves", &mOctaves).min(1).max(20).group("Noise Params").updateFn( [this] { console() << "new mOctaves value: " << mOctaves << endl; } );
    
    mParams->addParam("Height Multiplier", &mHeightMult ).group("Mesh Params");
    
    function<void( int )> planeSizeSetter = bind(&MeshParamTestApp::setPlaneSize, this, placeholders::_1);
    function<int ()> planeSizeGetter = bind(&MeshParamTestApp::getPlaneSize, this);
    mParams->addParam("Plane Size", planeSizeSetter, planeSizeGetter).group("Mesh Params");
    
    function<void( int )> planeSubdivisionsSetter = bind( &MeshParamTestApp::setPlaneSubdivisions, this, placeholders::_1 );
    function<int ()> planeSubdivisionsGetter = bind( &MeshParamTestApp::getPlaneSubdivisions, this );
    mParams->addParam( "Plane Subdivisions", planeSubdivisionsSetter, planeSubdivisionsGetter ).group("Mesh Params");
    
    mParams->addParam("Frequency", &mNoiseFrequency).min(0.1f).max(20.0f).group("Fractal Params").updateFn([this]{updateNoise();});
    mParams->addParam("Amplitude", &mNoiseAmplitude).min(0.1f).max(20.0f).group("Fractal Params").updateFn([this]{updateNoise();});
    mParams->addParam("Lacunarity", &mNoiseLacunarity).min(0.1f).max(20.0f).group("Fractal Params").updateFn([this]{updateNoise();});
    mParams->addParam("Persistence", &mNoisePersistence).min(0.1f).max(20.0f).group("Fractal Params").updateFn([this]{updateNoise();});
}

void MeshParamTestApp::resize()
{
    mCamera.setAspectRatio( getWindowAspectRatio() );
}

void MeshParamTestApp::update()
{
    udpatePlaneHeights();
}

void MeshParamTestApp::draw()
{
    // this pair of lines is the standard way to clear the screen in OpenGL
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    gl::clear( Color::gray( 0.1f ) );
    
    gl::setMatrices( mCamera );
    mCamera.lookAt(mCameraEyePoint, mCameraTarget);
    gl::rotate( mObjOrientation );
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    gl::drawCube( vec3( 0 ), vec3( mObjSize ) );
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    // Draw the interface
    mParams->draw();
    
    gl::ScopedGlslProg glslScope( gl::getStockShader( gl::ShaderDef().texture() ) );
    gl::ScopedTextureBind texScope( mTexture );
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    gl::draw( mVboMesh );
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

CINDER_APP( MeshParamTestApp, RendererGl, prepareSettings )
