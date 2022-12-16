
#include "ofxAutoReloadedShader.h"



ofxAutoReloadedShader::ofxAutoReloadedShader(){
	bWatchingFiles = false;
}

ofxAutoReloadedShader::~ofxAutoReloadedShader(){
	disableWatchFiles();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
bool ofxAutoReloadedShader::load(string shaderName )
{
	return load( shaderName + ".vert", shaderName + ".frag", shaderName + ".geom" );
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
bool ofxAutoReloadedShader::load(string vertName, string fragName, string geomName)
{
	unload();
	
    ofShader::setGeometryOutputCount(geometryOutputCount);
    ofShader::setGeometryInputType(geometryInputType);
    ofShader::setGeometryOutputType(geometryOutputType);

    
	// hackety hack, clear errors or shader will fail to compile
	GLuint err = glGetError();
	
	lastTimeCheckMillis = ofGetElapsedTimeMillis();
	setMillisBetweenFileCheck( 2 * 1000 );
	enableWatchFiles();
	
	loadShaderNextFrame = false;
	
	vertexShaderFilename = vertName;
	fragmentShaderFilename = fragName;
	geometryShaderFilename = geomName;
	
	vertexShaderFile.clear();
	fragmentShaderFile.clear();
	geometryShaderFile.clear();
	
	vertexShaderFile   = ofFile( ofToDataPath( vertexShaderFilename ) );
	fragmentShaderFile = ofFile( ofToDataPath( fragmentShaderFilename ) );
	geometryShaderFile = ofFile( ofToDataPath( geometryShaderFilename ) );
	
	fileChangedTimes.clear();
	fileChangedTimes.push_back( getLastModified( vertexShaderFile ) );
	fileChangedTimes.push_back( getLastModified( fragmentShaderFile ) );
	fileChangedTimes.push_back( getLastModified( geometryShaderFile ) );

    setupShaderFromFile(GL_VERTEX_SHADER, vertexShaderFilename);
    setupShaderFromFile(GL_FRAGMENT_SHADER, fragmentShaderFilename);

	#ifndef TARGET_OPENGLES
    if (geometryShaderFilename.empty() == false) {
        setupShaderFromFile(GL_GEOMETRY_SHADER_EXT, geometryShaderFilename);
    }
	#endif

	bindDefaults();
	
	return linkProgram();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxAutoReloadedShader::_update(ofEventArgs &e)
{
	if( loadShaderNextFrame )
	{
		reloadShaders();
		loadShaderNextFrame = false;
	}
	
	int currTime = ofGetElapsedTimeMillis();
	
	if( ((currTime - lastTimeCheckMillis) > millisBetweenFileCheck) &&
	   !loadShaderNextFrame )
	{
		if( filesChanged() )
		{
			loadShaderNextFrame = true;
		}
		
		lastTimeCheckMillis = currTime;
	}
}


// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
bool ofxAutoReloadedShader::reloadShaders()
{
	return load( vertexShaderFilename,  fragmentShaderFilename, geometryShaderFilename );
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxAutoReloadedShader::enableWatchFiles()
{
	if(!bWatchingFiles){
		ofAddListener(ofEvents().update, this, &ofxAutoReloadedShader::_update );
		bWatchingFiles = true;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxAutoReloadedShader::disableWatchFiles()
{
	if(bWatchingFiles){
		ofRemoveListener(ofEvents().update, this, &ofxAutoReloadedShader::_update );
		bWatchingFiles = false;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
bool ofxAutoReloadedShader::filesChanged()
{
	bool fileChanged = false;
	
	if( vertexShaderFile.exists() )
	{
		std::time_t vertexShaderFileLastChangeTime = getLastModified( vertexShaderFile );
		if( vertexShaderFileLastChangeTime != fileChangedTimes.at(0) )
		{
			fileChangedTimes.at(0) = vertexShaderFileLastChangeTime;
			fileChanged = true;
		}
	}
	
	if( fragmentShaderFile.exists() )
	{
		std::time_t fragmentShaderFileLastChangeTime = getLastModified( fragmentShaderFile );
		if( fragmentShaderFileLastChangeTime != fileChangedTimes.at(1) )
		{
			fileChangedTimes.at(1) = fragmentShaderFileLastChangeTime;
			fileChanged = true;
		}
	}
	
	
	if( geometryShaderFile.exists() )
	{
		std::time_t geometryShaderFileLastChangeTime = getLastModified( geometryShaderFile );
		if( geometryShaderFileLastChangeTime != fileChangedTimes.at(2) )
		{
			fileChangedTimes.at(2) = geometryShaderFileLastChangeTime;
			fileChanged = true;
		}
	}
	
	return fileChanged;
}

template <typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
              + system_clock::now());
    return system_clock::to_time_t(sctp);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
std::time_t ofxAutoReloadedShader::getLastModified( ofFile& _file )
{
	if( _file.exists() )
	{
#if OF_VERSION_MAJOR==0 && OF_VERSION_MINOR<=11
#if OF_VERSION_MINOR<11 || OF_VERSION_PATCH <= 1
        // <= 0.11.1
        return std::filesystem::last_write_time(_file.path());
#else
        // > 0.11.1
        const auto ftime = std::filesystem::last_write_time(_file.path());
        return to_time_t(ftime);
#endif
#else
        // > 0.11
        const auto ftime = std::filesystem::last_write_time(_file.path());
        return to_time_t(ftime);
#endif
	}
	else
	{
		return 0;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
//
void ofxAutoReloadedShader::setMillisBetweenFileCheck( int _millis )
{
	millisBetweenFileCheck = _millis;
}

//--------------------------------------------------------------
void ofxAutoReloadedShader::setGeometryInputType(GLenum type) {
    ofShader::setGeometryInputType(type);
    geometryInputType = type;
}

//--------------------------------------------------------------
void ofxAutoReloadedShader::setGeometryOutputType(GLenum type) {
    ofShader::setGeometryOutputType(type);
    geometryOutputType = type;
}

//--------------------------------------------------------------
void ofxAutoReloadedShader::setGeometryOutputCount(int count) {
    ofShader::setGeometryOutputCount(count);
    geometryOutputCount = count;
}
