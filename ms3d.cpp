
#include "sys/includes.h"
#include "ms3d.h"

//int m_frame = 0;

MS3DModel::MS3DModel()
{
	m_numMeshes = 0;
	m_pMeshes = NULL;
	m_numMaterials = 0;
	m_pMaterials = NULL;
	m_numTriangles = 0;
	m_pTriangles = NULL;
	m_numVertices = 0;
	m_pVertices = NULL;
	m_numJoints = 0;
	m_pJoints = NULL;
}

MS3DModel::~MS3DModel()
{
	destroy();
}

void MS3DModel::destroy()
{
	int i;
	for ( i = 0; i < m_numMeshes; i++ )
		delete[] m_pMeshes[i].m_pTriangleIndices;
	for ( i = 0; i < m_numMaterials; i++ )
		delete[] m_pMaterials[i].m_pTextureFilename;

	m_numMeshes = 0;
	if ( m_pMeshes != NULL )
	{
		delete[] m_pMeshes;
		m_pMeshes = NULL;
	}

	m_numMaterials = 0;
	if ( m_pMaterials != NULL )
	{
		delete[] m_pMaterials;
		m_pMaterials = NULL;
	}

	m_numTriangles = 0;
	if ( m_pTriangles != NULL )
	{
		delete[] m_pTriangles;
		m_pTriangles = NULL;
	}

	m_numVertices = 0;
	if ( m_pVertices != NULL )
	{
		delete[] m_pVertices;
		m_pVertices = NULL;
	}

	m_numJoints = 0;
	if(m_pJoints != NULL)
	{
		delete [] m_pJoints;
		m_pJoints = NULL;
	}
}

bool MS3DModel::rewrite(const char *relative, unsigned int& diffm, unsigned int& specm, unsigned int& normm, unsigned int& ownm, bool dontqueue)
{
	//char full[SPE_MAX_PATH+1];
	//FullPath(relative, full);
	
	FILE* fp = fopen(relative, "rb");

	//std::ifstream inputFile(relative, std::ios::in | std::ios::binary );
	//if ( inputFile.fail())
	if (!fp)
	{
		//Log("Couldn't show the model file %s ", relative);

		//char msg[SPE_MAX_PATH+1];
		//sprintf(msg, "Couldn't show the model file %s", relative);

		//ErrMess("Error", msg);
		printf("failed to load model\r\n");

		return false;
	}

	//std::string reltemp = StripFile(relative);

	//if(strlen(reltemp.c_str()) == 0)
	//	reltemp += CORRECT_SLASH;

	//strcpy(m_relative, reltemp.c_str());
	strcpy(m_relative, relative);

	/*
	char pathTemp[SPE_MAX_PATH+1];
	int pathLength;
	for ( pathLength = strlen( filename ); --pathLength; )
	{
		if ( filename[pathLength] == '/' || filename[pathLength] == '\\' )
			break;
	}
	strncpy( pathTemp, filename, pathLength );

	int i;
	if ( pathLength > 0 )
	{
		pathTemp[pathLength++] = '/';
	}

	strncpy( m_filepath, filename, pathLength );
	*/

	//inputFile.seekg( 0, std::ios::end );
	fseek(fp, 0, SEEK_END);
	//long fileSize = inputFile.tellg();
	long fileSize = ftell(fp);
	//inputFile.seekg( 0, std::ios::beg );
	fseek(fp, 0, SEEK_SET);

	char *pBuffer = new char[fileSize];
	//inputFile.read( pBuffer, fileSize );
	//inputFile.close();
	fread(pBuffer, 1, fileSize, fp);
	fclose(fp);

	char *pPtr = pBuffer;
	MS3DHeader *pHeader = ( MS3DHeader* )pPtr;
	pPtr += sizeof( MS3DHeader );

	if ( strncmp( pHeader->m_ID, "MS3D000000", 10 ) != 0 )
	{
		//Log("Not an MS3D file %s", relative);
		printf("not a model %s %d [%d] %d\r\n", pHeader->m_ID, sizeof(MS3DHeader), (int) pHeader->m_ID[4], fileSize);
		return false;
    }

	if ( pHeader->m_version < 3 )
	{
		//Log("I know nothing about MS3D v1.2, %s" , relative);

		//char msg[SPE_MAX_PATH+1];
		//sprintf(msg, "Incompatible MS3D v1.2 ", relative);

		//ErrMess("Error", msg);
		printf("model is 1.2\r\n");

		return false;
	}

	int nVertices = *( word* )pPtr;
	m_numVertices = nVertices;
	m_pVertices = new Vertex[nVertices];
	pPtr += sizeof( word );
	char* vstart = pPtr;

	int i;
	for ( i = 0; i < nVertices; i++ )
	{
		MS3DVertex *pVertex = ( MS3DVertex* )pPtr;
		m_pVertices[i].m_boneID = pVertex->m_boneID;
		memcpy( m_pVertices[i].m_location, pVertex->m_vertex, sizeof( float )*3 );
		pPtr += sizeof( MS3DVertex );
	}

	int nTriangles = *( word* )pPtr;
	m_numTriangles = nTriangles;
	m_pTriangles = new Triangle[nTriangles];
	pPtr += sizeof( word );

	for ( i = 0; i < nTriangles; i++ )
	{
		MS3DTriangle *pTriangle = ( MS3DTriangle* )pPtr;
		int vertexIndices[3] = { pTriangle->m_vertexIndices[0], pTriangle->m_vertexIndices[1], pTriangle->m_vertexIndices[2] };
		float t[3] = { 1.0f-pTriangle->m_t[0], 1.0f-pTriangle->m_t[1], 1.0f-pTriangle->m_t[2] };
		memcpy( m_pTriangles[i].m_vertexNormals, pTriangle->m_vertexNormals, sizeof( float )*3*3 );
		memcpy( m_pTriangles[i].m_s, pTriangle->m_s, sizeof( float )*3 );
		memcpy( m_pTriangles[i].m_t, t, sizeof( float )*3 );
		memcpy( m_pTriangles[i].m_vertexIndices, vertexIndices, sizeof( int )*3 );
		pPtr += sizeof( MS3DTriangle );
	}

	int nGroups = *( word* )pPtr;
	m_numMeshes = nGroups;
	m_pMeshes = new Mesh[nGroups];
	pPtr += sizeof( word );
	for ( i = 0; i < nGroups; i++ )
	{
		pPtr += sizeof( byte );	// flags
		pPtr += 32;				// name

		word nTriangles = *( word* )pPtr;
		pPtr += sizeof( word );
		int *pTriangleIndices = new int[nTriangles];
		for ( int j = 0; j < nTriangles; j++ )
		{
			pTriangleIndices[j] = *( word* )pPtr;
			pPtr += sizeof( word );
		}

		char materialIndex = *( char* )pPtr;
		pPtr += sizeof( char );

		m_pMeshes[i].m_materialIndex = materialIndex;
		m_pMeshes[i].m_numTriangles = nTriangles;
		m_pMeshes[i].m_pTriangleIndices = pTriangleIndices;
	}

	int nMaterials = *( word* )pPtr;
	m_numMaterials = nMaterials;
	m_pMaterials = new Material[nMaterials];
	pPtr += sizeof( word );
	for ( i = 0; i < nMaterials; i++ )
	{
		MS3DMaterial *pMaterial = ( MS3DMaterial* )pPtr;
		memcpy( m_pMaterials[i].m_ambient, pMaterial->m_ambient, sizeof( float )*4 );
		memcpy( m_pMaterials[i].m_diffuse, pMaterial->m_diffuse, sizeof( float )*4 );
		memcpy( m_pMaterials[i].m_specular, pMaterial->m_specular, sizeof( float )*4 );
		memcpy( m_pMaterials[i].m_emissive, pMaterial->m_emissive, sizeof( float )*4 );
		m_pMaterials[i].m_shininess = pMaterial->m_shininess;
		if ( strncmp( pMaterial->m_diffusem, ".\\", 2 ) == 0 ) {
			// MS3D 1.5.x relative path
			//StripPath(pMaterial->m_diffusem);
			//m_pMaterials[i].m_pTextureFilename = new char[ strlen(relativepath.c_str()) + strlen(pMaterial->m_diffusem) + 1 ];
			//strcpy( m_pMaterials[i].m_pTextureFilename, reltemp.c_str() );
			//sprintf(m_pMaterials[i].m_pTextureFilename, "%s%s", relativepath.c_str(), pMaterial->m_diffusem);
			m_pMaterials[i].m_pTextureFilename = new char[strlen( pMaterial->m_diffusem )+1];
			strcpy( m_pMaterials[i].m_pTextureFilename, pMaterial->m_diffusem );
		}
		else {
			// MS3D 1.4.x or earlier - absolute path
			m_pMaterials[i].m_pTextureFilename = new char[strlen( pMaterial->m_diffusem )+1];
			strcpy( m_pMaterials[i].m_pTextureFilename, pMaterial->m_diffusem );
		}
		//StripPath(m_pMaterials[i].m_pTextureFilename);
		pPtr += sizeof( MS3DMaterial );
	}

	//loadtex(diffm, specm, normm, ownm, dontqueue);

	float animFPS = *( float* )pPtr;
	pPtr += sizeof( float );

	// skip currentTime
	pPtr += sizeof( float );

	m_totalFrames = *( int* )pPtr;
	pPtr += sizeof( int );

	m_totalTime = m_totalFrames*1000.0/animFPS;

	m_numJoints = *( word* )pPtr;
	pPtr += sizeof( word );

	m_pJoints = new Joint[m_numJoints];

	struct JointNameListRec
	{
		int m_jointIndex;
		const char *m_pName;
	};

	const char *pTempPtr = pPtr;

	JointNameListRec *pNameList = new JointNameListRec[m_numJoints];
	for ( i = 0; i < m_numJoints; i++ )
	{
		MS3DJoint *pJoint = ( MS3DJoint* )pTempPtr;
		pTempPtr += sizeof( MS3DJoint );
		pTempPtr += sizeof( MS3DKeyframe )*( pJoint->m_numRotationKeyframes+pJoint->m_numTranslationKeyframes );

		pNameList[i].m_jointIndex = i;
		pNameList[i].m_pName = pJoint->m_name;
	}

	for ( i = 0; i < m_numJoints; i++ )
	{
		MS3DJoint *pJoint = ( MS3DJoint* )pPtr;
		pPtr += sizeof( MS3DJoint );

		int j, parentIndex = -1;
		if ( strlen( pJoint->m_parentName ) > 0 )
		{
			for ( j = 0; j < m_numJoints; j++ )
			{
				if ( _stricmp( pNameList[j].m_pName, pJoint->m_parentName ) == 0 )
				{
					parentIndex = pNameList[j].m_jointIndex;
					break;
				}
			}
			if ( parentIndex == -1 ) {
				//Log("Unable to find parent bone in MS3D file");
				printf("failed to load model bone\r\n");
				return false;
			}
		}

		memcpy( m_pJoints[i].m_localRotation, pJoint->m_rotation, sizeof( float )*3 );
		memcpy( m_pJoints[i].m_localTranslation, pJoint->m_translation, sizeof( float )*3 );
		m_pJoints[i].m_parent = parentIndex;
		m_pJoints[i].m_numRotationKeyframes = pJoint->m_numRotationKeyframes;
		m_pJoints[i].m_pRotationKeyframes = new Keyframe[pJoint->m_numRotationKeyframes];
		m_pJoints[i].m_numTranslationKeyframes = pJoint->m_numTranslationKeyframes;
		m_pJoints[i].m_pTranslationKeyframes = new Keyframe[pJoint->m_numTranslationKeyframes];

		for ( j = 0; j < pJoint->m_numRotationKeyframes; j++ )
		{
			MS3DKeyframe *pKeyframe = ( MS3DKeyframe* )pPtr;
			pPtr += sizeof( MS3DKeyframe );

			setjointkf( i, j, pKeyframe->m_time*1000.0f, pKeyframe->m_parameter, true );
		}

		for ( j = 0; j < pJoint->m_numTranslationKeyframes; j++ )
		{
			MS3DKeyframe *pKeyframe = ( MS3DKeyframe* )pPtr;
			pPtr += sizeof( MS3DKeyframe );

			setjointkf( i, j, pKeyframe->m_time*1000.0f, pKeyframe->m_parameter, false );
		}
	}

	//count
#if 01
	float neard = -1, fard = 0;

	for (i = 0; i < nVertices; i++)
	{
		Vec3f v;
		v.x = m_pVertices[i].m_location[0];
		v.y = m_pVertices[i].m_location[1];
		v.z = m_pVertices[i].m_location[2];
		float d = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
		if (neard < 0 || d < neard)
			neard = d;
		if (d > fard)
			fard = d;
	}

	//rearrange

	for (i = 0; i < nVertices; i++)
	{
		Vec3f v;
		v.x = m_pVertices[i].m_location[0];
		v.y = m_pVertices[i].m_location[1];
		v.z = m_pVertices[i].m_location[2];
		float d = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
		v.x /= d;
		v.y /= d;
		v.z /= d;
		float newd = (fard + neard) - d;
		v.x *= newd;
		v.y *= newd;
		v.z *= newd;
		m_pVertices[i].m_location[0] = v.x;
		m_pVertices[i].m_location[1] = v.y;
		m_pVertices[i].m_location[2] = v.z;
	}
#endif
	//write
	for (i = 0; i < nVertices; i++)
	{
		((MS3DVertex*)(vstart + sizeof(MS3DVertex) * i))->m_vertex[0] = m_pVertices[i].m_location[0];
		((MS3DVertex*)(vstart + sizeof(MS3DVertex) * i))->m_vertex[1] = m_pVertices[i].m_location[1];
		((MS3DVertex*)(vstart + sizeof(MS3DVertex) * i))->m_vertex[2] = m_pVertices[i].m_location[2];
	}
	fp = fopen("out.ms3d", "wb");
	if(!fp)
		printf("failed to write model\r\n");
	fwrite(pBuffer, 1, fileSize, fp);
	fclose(fp);


	delete[] pNameList;

	//setupjoints();

	delete[] pBuffer;

	//restart();

	//Log(relative);

	return true;
}

void MS3DModel::setjointkf( int jointIndex, int keyframeIndex, float time, float *parameter, bool isRotation )
{
	Keyframe& keyframe = isRotation ? m_pJoints[jointIndex].m_pRotationKeyframes[keyframeIndex] :
		m_pJoints[jointIndex].m_pTranslationKeyframes[keyframeIndex];

	keyframe.m_jointIndex = jointIndex;
	keyframe.m_time = time;
	memcpy( keyframe.m_parameter, parameter, sizeof( float )*3 );
}

