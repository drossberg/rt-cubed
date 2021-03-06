/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreD3D9Texture.h"
#include "OgreD3D9HardwarePixelBuffer.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"
#include "OgreD3D9Mappings.h"
#include "OgreD3D9RenderSystem.h"
#include "OgreD3D9TextureManager.h"
#include "OgreRoot.h"
#include "OgreD3D9Device.h"
#include "OgreD3D9DeviceManager.h"
#include "OgreD3D9ResourceManager.h"

namespace Ogre 
{
	/****************************************************************************************/
    D3D9Texture::D3D9Texture(ResourceManager* creator, const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader)
        :Texture(creator, name, handle, group, isManual, loader),              
        mD3DPool(D3DPOOL_MANAGED),
		mDynamicTextures(false),
		mHwGammaReadSupported(false),
		mHwGammaWriteSupported(false),	
		mFSAAType(D3DMULTISAMPLE_NONE),
		mFSAAQuality(0)
	{
       
	}
	/****************************************************************************************/
	D3D9Texture::~D3D9Texture()
	{		
        // have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
		if (isLoaded())
		{
			unload(); 
		}
		else
		{
			freeInternalResources();
		}		

		// Free memory allocated per device.
		DeviceToTextureResourcesIterator it = mMapDeviceToTextureResources.begin();
		while (it != mMapDeviceToTextureResources.end())
		{
			TextureResources* textureResource = it->second;
			
			SAFE_DELETE(textureResource);
			++it;
		}		
		mMapDeviceToTextureResources.clear();
	}
	/****************************************************************************************/
	void D3D9Texture::copyToTexture(TexturePtr& target)
	{
        // check if this & target are the same format and type
		// blitting from or to cube textures is not supported yet
		if (target->getUsage() != getUsage() ||
			target->getTextureType() != getTextureType())
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
					"Src. and dest. textures must be of same type and must have the same usage !!!", 
					"D3D9Texture::copyToTexture" );
		}

        HRESULT hr;
        D3D9Texture *other;
		// get the target
		other = reinterpret_cast< D3D9Texture * >( target.get() );
		// target rectangle (whole surface)
		RECT dstRC = {0, 0, static_cast<LONG>(other->getWidth()), static_cast<LONG>(other->getHeight())};

		DeviceToTextureResourcesIterator it = mMapDeviceToTextureResources.begin();
		while (it != mMapDeviceToTextureResources.end())
		{
			TextureResources* srcTextureResource = it->second;
			TextureResources* dstTextureResource = other->getTextureResources(it->first);

	
			// do it plain for normal texture
			if (getTextureType() == TEX_TYPE_2D &&
				srcTextureResource->pNormTex && 
				dstTextureResource->pNormTex)
			{			
				// get our source surface
				IDirect3DSurface9 *pSrcSurface = 0;
				if( FAILED( hr = srcTextureResource->pNormTex->GetSurfaceLevel(0, &pSrcSurface) ) )
				{
					String msg = DXGetErrorDescription9(hr);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
				}

				// get our target surface
				IDirect3DSurface9 *pDstSurface = 0;			
				if( FAILED( hr = dstTextureResource->pNormTex->GetSurfaceLevel(0, &pDstSurface) ) )
				{
					String msg = DXGetErrorDescription9(hr);
					SAFE_RELEASE(pSrcSurface);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
				}

				// do the blit, it's called StretchRect in D3D9 :)
				if( FAILED( hr = it->first->StretchRect( pSrcSurface, NULL, pDstSurface, &dstRC, D3DTEXF_NONE) ) )
				{
					String msg = DXGetErrorDescription9(hr);
					SAFE_RELEASE(pSrcSurface);
					SAFE_RELEASE(pDstSurface);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
				}

				// release temp. surfaces
				SAFE_RELEASE(pSrcSurface);
				SAFE_RELEASE(pDstSurface);
			}
			else if (getTextureType() == TEX_TYPE_CUBE_MAP &&
				srcTextureResource->pCubeTex && 
				dstTextureResource->pCubeTex)
			{				
				// blit to 6 cube faces
				for (size_t face = 0; face < 6; face++)
				{
					// get our source surface
					IDirect3DSurface9 *pSrcSurface = 0;
					if( FAILED( hr =srcTextureResource->pCubeTex->GetCubeMapSurface((D3DCUBEMAP_FACES)face, 0, &pSrcSurface) ) )
					{
						String msg = DXGetErrorDescription9(hr);
						OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
					}

					// get our target surface
					IDirect3DSurface9 *pDstSurface = 0;
					if( FAILED( hr = dstTextureResource->pCubeTex->GetCubeMapSurface((D3DCUBEMAP_FACES)face, 0, &pDstSurface) ) )
					{
						String msg = DXGetErrorDescription9(hr);
						SAFE_RELEASE(pSrcSurface);
						OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
					}

					// do the blit, it's called StretchRect in D3D9 :)
					if( FAILED( hr = it->first->StretchRect( pSrcSurface, NULL, pDstSurface, &dstRC, D3DTEXF_NONE) ) )
					{
						String msg = DXGetErrorDescription9(hr);
						SAFE_RELEASE(pSrcSurface);
						SAFE_RELEASE(pDstSurface);
						OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
					}

					// release temp. surfaces
					SAFE_RELEASE(pSrcSurface);
					SAFE_RELEASE(pDstSurface);
				}
			}
			else
			{
				OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED, 
					"Copy to texture is implemented only for 2D and cube textures !!!", 
					"D3D9Texture::copyToTexture" );
			}

			++it;
		}		
	}
	/****************************************************************************************/
	void D3D9Texture::loadImpl()
	{
		if (!mInternalResourcesCreated)
		{
			// NB: Need to initialise pool to some value other than D3DPOOL_DEFAULT,
			// otherwise, if the texture loading failed, it might re-create as empty
			// texture when device lost/restore. The actual pool will determine later.
			mD3DPool = D3DPOOL_MANAGED;
		}

		for (uint i = 0; i < D3D9RenderSystem::getResourceCreationDeviceCount(); ++i)
		{
			IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getResourceCreationDevice(i);

			loadImpl(d3d9Device);
		}

		mLoadedStreams.setNull();
	}

	/****************************************************************************************/
	void D3D9Texture::loadImpl(IDirect3DDevice9* d3d9Device)
	{
		if (mUsage & TU_RENDERTARGET)
		{
			createInternalResourcesImpl(d3d9Device);
			return;
		}

		// only copy is on the stack so well-behaved if exception thrown
		LoadedStreams loadedStreams = mLoadedStreams;
	
		// load based on tex.type
		switch (getTextureType())
		{
		case TEX_TYPE_1D:
		case TEX_TYPE_2D:
			_loadNormTex(d3d9Device, loadedStreams);
			break;
		case TEX_TYPE_3D:
			_loadVolumeTex(d3d9Device, loadedStreams);
			break;
		case TEX_TYPE_CUBE_MAP:
			_loadCubeTex(d3d9Device, loadedStreams);
			break;
		default:
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D9Texture::loadImpl" );
		}

		for (uint i = 0; i < mLoadedStreams->size(); ++i)
		{
			MemoryDataStreamPtr curDataStream = (*mLoadedStreams)[i];

			curDataStream->seek(0);
		}
		

	}
	/****************************************************************************************/
	void D3D9Texture::prepareImpl()
	{
		if (mUsage & TU_RENDERTARGET)
		{
			return;
		}

        LoadedStreams loadedStreams;

		// prepare load based on tex.type
		switch (getTextureType())
		{
		case TEX_TYPE_1D:
		case TEX_TYPE_2D:
			loadedStreams = _prepareNormTex();
			break;
		case TEX_TYPE_3D:
            loadedStreams = _prepareVolumeTex();
            break;
		case TEX_TYPE_CUBE_MAP:
			loadedStreams = _prepareCubeTex();
			break;
		default:
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D9Texture::prepareImpl" );
		}

        mLoadedStreams = loadedStreams;
	}
    /****************************************************************************************/
	D3D9Texture::LoadedStreams D3D9Texture::_prepareCubeTex()
	{
		assert(getTextureType() == TEX_TYPE_CUBE_MAP);

        LoadedStreams loadedStreams = LoadedStreams(new vector<MemoryDataStreamPtr>::type());
        // DDS load?
		if (getSourceFileType() == "dds")
		{
            // find & load resource data
			DataStreamPtr dstream = 
				ResourceGroupManager::getSingleton().openResource(
					mName, mGroup, true, this);
            loadedStreams->push_back(MemoryDataStreamPtr(new MemoryDataStream(dstream)));
        }
        else
        {
			// Load from 6 separate files
			// Use OGRE its own codecs
			String baseName, ext;
			size_t pos = mName.find_last_of(".");
			baseName = mName.substr(0, pos);
			if ( pos != String::npos )
				ext = mName.substr(pos+1);
			static const String suffixes[6] = {"_rt", "_lf", "_up", "_dn", "_fr", "_bk"};

			for(size_t i = 0; i < 6; i++)
			{
				String fullName = baseName + suffixes[i];
				if (!ext.empty())
					fullName = fullName + "." + ext;

            	// find & load resource data intro stream to allow resource
				// group changes if required
				DataStreamPtr dstream = 
					ResourceGroupManager::getSingleton().openResource(
						fullName, mGroup, true, this);

                loadedStreams->push_back(MemoryDataStreamPtr(new MemoryDataStream(dstream)));
			}
        }

        return loadedStreams;
	}
	/****************************************************************************************/
	D3D9Texture::LoadedStreams D3D9Texture::_prepareVolumeTex()
	{
		assert(getTextureType() == TEX_TYPE_3D);

		// find & load resource data
		DataStreamPtr dstream = 
			ResourceGroupManager::getSingleton().openResource(
				mName, mGroup, true, this);

        LoadedStreams loadedStreams = LoadedStreams(new vector<MemoryDataStreamPtr>::type());
        loadedStreams->push_back(MemoryDataStreamPtr(new MemoryDataStream(dstream)));
        return loadedStreams;
    }
	/****************************************************************************************/
	D3D9Texture::LoadedStreams D3D9Texture::_prepareNormTex()
	{
		assert(getTextureType() == TEX_TYPE_1D || getTextureType() == TEX_TYPE_2D);

		// find & load resource data
		DataStreamPtr dstream = 
			ResourceGroupManager::getSingleton().openResource(
				mName, mGroup, true, this);

        LoadedStreams loadedStreams = LoadedStreams(new vector<MemoryDataStreamPtr>::type());
        loadedStreams->push_back(MemoryDataStreamPtr(new MemoryDataStream(dstream)));
        return loadedStreams;
	}
	/****************************************************************************************/
	void D3D9Texture::unprepareImpl()
	{
		if (mUsage & TU_RENDERTARGET)
		{
			return;
		}

        mLoadedStreams.setNull();

	}

	/****************************************************************************************/
	void D3D9Texture::freeInternalResources(void)
	{
		freeInternalResourcesImpl();
	}

	/****************************************************************************************/
	void D3D9Texture::freeInternalResourcesImpl()
	{
		DeviceToTextureResourcesIterator it = mMapDeviceToTextureResources.begin();

		while (it != mMapDeviceToTextureResources.end())
		{
			TextureResources* textureResource = it->second;

			freeTextureResources(it->first, textureResource);			
			++it;
		}						
	}

	/****************************************************************************************/
	D3D9Texture::TextureResources* D3D9Texture::getTextureResources(IDirect3DDevice9* d3d9Device)
	{
		DeviceToTextureResourcesIterator it = mMapDeviceToTextureResources.find(d3d9Device);

		if (it == mMapDeviceToTextureResources.end())		
			return NULL;		
		
		return it->second;
	}

	/****************************************************************************************/
	D3D9Texture::TextureResources* D3D9Texture::allocateTextureResources(IDirect3DDevice9* d3d9Device)
	{
		assert(mMapDeviceToTextureResources.find(d3d9Device) == mMapDeviceToTextureResources.end());

		TextureResources* textureResources = new TextureResources;

		textureResources->pNormTex		= NULL;
		textureResources->pCubeTex		= NULL;
		textureResources->pVolumeTex	= NULL;
		textureResources->pBaseTex		= NULL;
		textureResources->pFSAASurface	= NULL;

		mMapDeviceToTextureResources[d3d9Device] = textureResources;

		return textureResources;
	}

	/****************************************************************************************/
	void D3D9Texture::createTextureResources(IDirect3DDevice9* d3d9Device)
	{
		OGRE_LOCK_AUTO_MUTEX
		
		if (mIsManual)
		{
			preLoadImpl();

			// create the internal resources.
			createInternalResourcesImpl(d3d9Device);

			// Load from manual loader
			if (mLoader != NULL)
			{
				mLoader->loadResource(this);
			}			
			postLoadImpl();
		}
		else
		{			
			prepareImpl();
			preLoadImpl();							
			loadImpl(d3d9Device);
			postLoadImpl();
		}		
	}

	/****************************************************************************************/
	void D3D9Texture::freeTextureResources(IDirect3DDevice9* d3d9Device, D3D9Texture::TextureResources* textureResources)
	{
		// Release surfaces from each mip level.
		for(unsigned int i = 0; i < mSurfaceList.size(); ++i)
		{
			D3D9HardwarePixelBuffer* pixelBuffer = static_cast<D3D9HardwarePixelBuffer*>(mSurfaceList[i].get());

			pixelBuffer->releaseSurfaces(d3d9Device);			
		}
		
		// Release the rest of the resources.
		SAFE_RELEASE(textureResources->pBaseTex);
		SAFE_RELEASE(textureResources->pNormTex);
		SAFE_RELEASE(textureResources->pCubeTex);
		SAFE_RELEASE(textureResources->pVolumeTex);
		SAFE_RELEASE(textureResources->pFSAASurface);
	}

	/****************************************************************************************/
	void D3D9Texture::_loadCubeTex(IDirect3DDevice9* d3d9Device, const D3D9Texture::LoadedStreams &loadedStreams)
	{
		assert(getTextureType() == TEX_TYPE_CUBE_MAP);

        // DDS load?
		if (getSourceFileType() == "dds")
		{
            // find & load resource data
            assert(loadedStreams->size()==1);

			DWORD usage = 0;
			UINT numMips = (mNumRequestedMipmaps == MIP_UNLIMITED) ?
				D3DX_DEFAULT : mNumRequestedMipmaps + 1;
			
			D3D9Device* device = D3D9RenderSystem::getDeviceManager()->getDeviceFromD3D9Device(d3d9Device);
			const D3DCAPS9& rkCurCaps = device->getD3D9DeviceCaps();			

			// check if mip map volume textures are supported	
			if (!(rkCurCaps.TextureCaps & D3DPTEXTURECAPS_MIPCUBEMAP))
			{
				// no mip map support for this kind of textures :(
				mNumMipmaps = 0;
				numMips = 1;
			}

            // Determine D3D pool to use
            D3DPOOL pool;
            if (useDefaultPool())
            {
                pool = D3DPOOL_DEFAULT;
            }
            else
            {
                pool = D3DPOOL_MANAGED;
            }
				
			TextureResources* textureResources;			
		
			// Get or create new texture resources structure.
			textureResources = getTextureResources(d3d9Device);
			if (textureResources != NULL)
				freeTextureResources(d3d9Device, textureResources);
			else
				textureResources = allocateTextureResources(d3d9Device);

			HRESULT hr;

			hr = D3DXCreateCubeTextureFromFileInMemoryEx(
				d3d9Device,
				(*loadedStreams)[0]->getPtr(),
				static_cast<UINT>((*loadedStreams)[0]->size()),
				D3DX_DEFAULT, // dims (square)
				numMips,
				usage,
				D3DFMT_UNKNOWN,
				pool,
				D3DX_DEFAULT,
				D3DX_DEFAULT,
				0,  // colour key
				NULL, // src box
				NULL, // palette
				&textureResources->pCubeTex); 

            if (FAILED(hr))
		    {
				freeInternalResources();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't create cube texture: " + String(DXGetErrorDescription9(hr)), 
					"D3D9Texture::_loadCubeTex" );
		    }

            hr = textureResources->pCubeTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&textureResources->pBaseTex);

            if (FAILED(hr))
		    {
				freeInternalResources();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't get base texture: " + String(DXGetErrorDescription9(hr)), 
					"D3D9Texture::_loadCubeTex" );
		    }

            D3DSURFACE_DESC texDesc;
            textureResources->pCubeTex->GetLevelDesc(0, &texDesc);
            mD3DPool = texDesc.Pool;
            // set src and dest attributes to the same, we can't know
            _setSrcAttributes(texDesc.Width, texDesc.Height, 1, D3D9Mappings::_getPF(texDesc.Format));
            _setFinalAttributes(d3d9Device, textureResources, 
				texDesc.Width, texDesc.Height, 1,  D3D9Mappings::_getPF(texDesc.Format));
			mInternalResourcesCreated = true;
        }
        else
        {
            assert(loadedStreams->size()==6);

			String  ext;
			size_t pos = mName.find_last_of(".");
			if ( pos != String::npos )
				ext = mName.substr(pos+1);

			vector<Image>::type images(6);
			ConstImagePtrList imagePtrs;

			for(size_t i = 0; i < 6; i++)
			{
				DataStreamPtr stream((*loadedStreams)[i]);
				images[i].load(stream, ext);

				imagePtrs.push_back(&images[i]);
			}

            _loadImages( imagePtrs );
        }
	}
	/****************************************************************************************/
	void D3D9Texture::_loadVolumeTex(IDirect3DDevice9* d3d9Device, const D3D9Texture::LoadedStreams &loadedStreams)
	{
		assert(getTextureType() == TEX_TYPE_3D);
		// DDS load?
		if (getSourceFileType() == "dds")
		{
			// find & load resource data
            assert(loadedStreams->size()==1);
	
			DWORD usage = 0;
			UINT numMips = (mNumRequestedMipmaps == MIP_UNLIMITED) ?
				D3DX_DEFAULT : mNumRequestedMipmaps + 1;
						
			D3D9Device* device = D3D9RenderSystem::getDeviceManager()->getDeviceFromD3D9Device(d3d9Device);
			const D3DCAPS9& rkCurCaps = device->getD3D9DeviceCaps();			

			// check if mip map volume textures are supported
			if (!(rkCurCaps.TextureCaps & D3DPTEXTURECAPS_MIPVOLUMEMAP))
			{
				// no mip map support for this kind of textures :(
				mNumMipmaps = 0;
				numMips = 1;
			}

            // Determine D3D pool to use
            D3DPOOL pool;
            if (useDefaultPool())
            {
                pool = D3DPOOL_DEFAULT;
            }
            else
            {
                pool = D3DPOOL_MANAGED;
            }
			
			TextureResources* textureResources;			
			
			// Get or create new texture resources structure.
			textureResources = getTextureResources(d3d9Device);
			if (textureResources != NULL)
				freeTextureResources(d3d9Device, textureResources);
			else
				textureResources = allocateTextureResources(d3d9Device);

			HRESULT hr;

			hr = D3DXCreateVolumeTextureFromFileInMemoryEx(
				d3d9Device,
				(*loadedStreams)[0]->getPtr(),
				static_cast<UINT>((*loadedStreams)[0]->size()),
				D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, // dims
				numMips,
				usage,
				D3DFMT_UNKNOWN,
				pool,
				D3DX_DEFAULT,
				D3DX_DEFAULT,
				0,  // colour key
				NULL, // src box
				NULL, // palette
				&textureResources->pVolumeTex); 
	
			if (FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Unable to load volume texture from " + getName() + ": " + String(DXGetErrorDescription9(hr)),
					"D3D9Texture::_loadVolumeTex");
			}
	
			hr = textureResources->pVolumeTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&textureResources->pBaseTex);
	
			if (FAILED(hr))
			{
				freeInternalResources();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't get base texture: " + String(DXGetErrorDescription9(hr)), 
					"D3D9Texture::_loadVolumeTex" );
			}
	
			D3DVOLUME_DESC texDesc;
			hr = textureResources->pVolumeTex->GetLevelDesc(0, &texDesc);
            mD3DPool = texDesc.Pool;
			// set src and dest attributes to the same, we can't know
			_setSrcAttributes(texDesc.Width, texDesc.Height, texDesc.Depth, D3D9Mappings::_getPF(texDesc.Format));
			_setFinalAttributes(d3d9Device, textureResources, 
				texDesc.Width, texDesc.Height, texDesc.Depth, D3D9Mappings::_getPF(texDesc.Format));
			mInternalResourcesCreated = true;
        }
		else
		{
			Image img;

            assert(loadedStreams->size()==1);

			size_t pos = mName.find_last_of(".");
			String ext;
			if ( pos != String::npos )
				ext = mName.substr(pos+1);
	
			DataStreamPtr stream((*loadedStreams)[0]);
			img.load(stream, ext);
			// Call internal _loadImages, not loadImage since that's external and 
			// will determine load status etc again
			ConstImagePtrList imagePtrs;
			imagePtrs.push_back(&img);
			_loadImages( imagePtrs );
		}
    }
	/****************************************************************************************/
	void D3D9Texture::_loadNormTex(IDirect3DDevice9* d3d9Device, const D3D9Texture::LoadedStreams &loadedStreams)
	{
		assert(getTextureType() == TEX_TYPE_1D || getTextureType() == TEX_TYPE_2D);
		// DDS load?
		if (getSourceFileType() == "dds")
		{
			// Use D3DX
            assert(loadedStreams->size()==1);
	
			DWORD usage = 0;
			UINT numMips;

			if (mNumRequestedMipmaps == MIP_UNLIMITED)
				numMips = D3DX_DEFAULT;
			else if (mNumRequestedMipmaps == 0)			
				numMips = D3DX_FROM_FILE;
			else
				numMips = static_cast<UINT>(mNumRequestedMipmaps + 1);
				
			D3D9Device* device = D3D9RenderSystem::getDeviceManager()->getDeviceFromD3D9Device(d3d9Device);
			const D3DCAPS9& rkCurCaps = device->getD3D9DeviceCaps();			


			// check if mip map volume textures are supported
			if (!(rkCurCaps.TextureCaps & D3DPTEXTURECAPS_MIPMAP))
			{
				// no mip map support for this kind of textures :(
				mNumMipmaps = 0;
				numMips = 1;
			}

            // Determine D3D pool to use
            D3DPOOL pool;
            if (useDefaultPool())
            {
                pool = D3DPOOL_DEFAULT;
            }
            else
            {
                pool = D3DPOOL_MANAGED;
            }
						
			TextureResources* textureResources;			
			
			// Get or create new texture resources structure.
			textureResources = getTextureResources(d3d9Device);
			if (textureResources != NULL)
				freeTextureResources(d3d9Device, textureResources);
			else
				textureResources = allocateTextureResources(d3d9Device);

			HRESULT hr;

			hr = D3DXCreateTextureFromFileInMemoryEx(
				d3d9Device,
				(*loadedStreams)[0]->getPtr(),
				static_cast<UINT>((*loadedStreams)[0]->size()),
				D3DX_DEFAULT, D3DX_DEFAULT, // dims
				numMips,
				usage,
				D3DFMT_UNKNOWN,
				pool,
				D3DX_DEFAULT,
				D3DX_DEFAULT,
				0,  // colour key
				NULL, // src box
				NULL, // palette
				&textureResources->pNormTex); 
	
			if (FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Unable to load texture from " + getName() + " :" + String(DXGetErrorDescription9(hr)),
					"D3D9Texture::_loadNormTex");
			}
	
			hr = textureResources->pNormTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&textureResources->pBaseTex);
	
			if (FAILED(hr))
			{
				freeInternalResources();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't get base texture: " + String(DXGetErrorDescription9(hr)), 
					"D3D9Texture::_loadNormTex" );
			}
	
			D3DSURFACE_DESC texDesc;
			textureResources->pNormTex->GetLevelDesc(0, &texDesc);
            mD3DPool = texDesc.Pool;
			// set src and dest attributes to the same, we can't know
			_setSrcAttributes(texDesc.Width, texDesc.Height, 1, D3D9Mappings::_getPF(texDesc.Format));
			_setFinalAttributes(d3d9Device, textureResources, 
				texDesc.Width, texDesc.Height, 1, D3D9Mappings::_getPF(texDesc.Format));
			mInternalResourcesCreated = true;
        }
		else
		{
			Image img;
           	// find & load resource data intro stream to allow resource
			// group changes if required
            assert(loadedStreams->size()==1);
	
			size_t pos = mName.find_last_of(".");
			String ext; 
			if ( pos != String::npos )
				ext = mName.substr(pos+1);

			DataStreamPtr stream((*loadedStreams)[0]);
			img.load(stream, ext);
			// Call internal _loadImages, not loadImage since that's external and 
			// will determine load status etc again
			ConstImagePtrList imagePtrs;
			imagePtrs.push_back(&img);
			_loadImages( imagePtrs );
		}

	}

	/****************************************************************************************/
	size_t D3D9Texture::calculateSize(void) const
	{
		size_t instanceSize = getNumFaces() * PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);

		return instanceSize * mMapDeviceToTextureResources.size();
	}

	/****************************************************************************************/
	void D3D9Texture::createInternalResources(void)
	{
		createInternalResourcesImpl();
	}

	/****************************************************************************************/
	void D3D9Texture::determinePool()
	{
		if (useDefaultPool())
		{
			mD3DPool = D3DPOOL_DEFAULT;
		}
		else
		{
			mD3DPool = D3DPOOL_MANAGED;
		}

	}
	/****************************************************************************************/
    void D3D9Texture::createInternalResourcesImpl(void)
	{
		for (uint i = 0; i < D3D9RenderSystem::getResourceCreationDeviceCount(); ++i)
		{
			IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getResourceCreationDevice(i);

			createInternalResourcesImpl(d3d9Device);
		}
	}

	/****************************************************************************************/
	void D3D9Texture::createInternalResourcesImpl(IDirect3DDevice9* d3d9Device)
	{
		TextureResources* textureResources;			

		// Check if resources already exist.
		textureResources = getTextureResources(d3d9Device);
		if (textureResources != NULL && textureResources->pBaseTex != NULL)
			return;
		

		// If mSrcWidth and mSrcHeight are zero, the requested extents have probably been set
		// through setWidth and setHeight, which set mWidth and mHeight. Take those values.
		if(mSrcWidth == 0 || mSrcHeight == 0) {
			mSrcWidth = mWidth;
			mSrcHeight = mHeight;
		}

		// load based on tex.type
		switch (getTextureType())
		{
		case TEX_TYPE_1D:
		case TEX_TYPE_2D:
			_createNormTex(d3d9Device);
			break;
		case TEX_TYPE_CUBE_MAP:
			_createCubeTex(d3d9Device);
			break;
		case TEX_TYPE_3D:
			_createVolumeTex(d3d9Device);
			break;
		default:
			freeInternalResources();
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D9Texture::createInternalResources" );
		}
	}

	/****************************************************************************************/
	void D3D9Texture::_createNormTex(IDirect3DDevice9* d3d9Device)
	{
		// we must have those defined here
		assert(mSrcWidth > 0 || mSrcHeight > 0);

		// determine wich D3D9 pixel format we'll use
		HRESULT hr;
		D3DFORMAT d3dPF = _chooseD3DFormat(d3d9Device);

		// let's D3DX check the corrected pixel format
		hr = D3DXCheckTextureRequirements(d3d9Device, NULL, NULL, NULL, 0, &d3dPF, mD3DPool);

		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		DWORD usage = (mUsage & TU_RENDERTARGET) ? D3DUSAGE_RENDERTARGET : 0;
		UINT numMips = (mNumRequestedMipmaps == MIP_UNLIMITED) ? 
				D3DX_DEFAULT : mNumRequestedMipmaps + 1;
		// Check dynamic textures
		if (mUsage & TU_DYNAMIC)
		{
			if (_canUseDynamicTextures(d3d9Device, usage, D3DRTYPE_TEXTURE, d3dPF))
			{
				usage |= D3DUSAGE_DYNAMIC;
				mDynamicTextures = true;
			}
			else
			{
				mDynamicTextures = false;
			}
		}
		// Check sRGB support
		if (mHwGamma)
		{
			mHwGammaReadSupported = _canUseHardwareGammaCorrection(d3d9Device, usage, D3DRTYPE_TEXTURE, d3dPF, false);
			if (mUsage & TU_RENDERTARGET)
				mHwGammaWriteSupported = _canUseHardwareGammaCorrection(d3d9Device, usage, D3DRTYPE_TEXTURE, d3dPF, true);
		}
		// Check FSAA level
		if (mUsage & TU_RENDERTARGET)
		{
			D3D9RenderSystem* rsys = static_cast<D3D9RenderSystem*>(Root::getSingleton().getRenderSystem());
			rsys->determineFSAASettings(d3d9Device, mFSAA, mFSAAHint, d3dPF, false, 
				&mFSAAType, &mFSAAQuality);
		}
		else
		{
			mFSAAType = D3DMULTISAMPLE_NONE;
			mFSAAQuality = 0;
		}

		D3D9Device* device = D3D9RenderSystem::getDeviceManager()->getDeviceFromD3D9Device(d3d9Device);
		const D3DCAPS9& rkCurCaps = device->getD3D9DeviceCaps();			


		// check if mip maps are supported on hardware
		mMipmapsHardwareGenerated = false;
		if (rkCurCaps.TextureCaps & D3DPTEXTURECAPS_MIPMAP)
		{
			if (mUsage & TU_AUTOMIPMAP && mNumRequestedMipmaps != 0)
			{
				// use auto.gen. if available, and if desired
				mMipmapsHardwareGenerated = _canAutoGenMipmaps(d3d9Device, usage, D3DRTYPE_TEXTURE, d3dPF);
				if (mMipmapsHardwareGenerated)
				{
					usage |= D3DUSAGE_AUTOGENMIPMAP;
					numMips = 0;
				}
			}
		}
		else
		{
			// no mip map support for this kind of textures :(
			mNumMipmaps = 0;
			numMips = 1;
		}

		// derive the pool to use
		determinePool();

		TextureResources* textureResources;			
	
		// Get or create new texture resources structure.
		textureResources = getTextureResources(d3d9Device);
		if (textureResources != NULL)
			freeTextureResources(d3d9Device, textureResources);
		else
			textureResources = allocateTextureResources(d3d9Device);


		// create the texture
		hr = D3DXCreateTexture(	
				d3d9Device,								// device
				static_cast<UINT>(mSrcWidth),			// width
				static_cast<UINT>(mSrcHeight),			// height
				numMips,								// number of mip map levels
				usage,									// usage
				d3dPF,									// pixel format
				mD3DPool,
				&textureResources->pNormTex);			// data pointer
		// check result and except if failed
		if (FAILED(hr))
		{
			freeInternalResources();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error creating texture: " + String(DXGetErrorDescription9(hr)), 
				"D3D9Texture::_createNormTex" );
		}
		
		// set the base texture we'll use in the render system
		hr = textureResources->pNormTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&textureResources->pBaseTex);
		if (FAILED(hr))
		{
			freeInternalResources();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't get base texture: " + String(DXGetErrorDescription9(hr)), 
				"D3D9Texture::_createNormTex" );
		}
		
		// set final tex. attributes from tex. description
		// they may differ from the source image !!!
		D3DSURFACE_DESC desc;
		hr = textureResources->pNormTex->GetLevelDesc(0, &desc);
		if (FAILED(hr))
		{
			freeInternalResources();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't get texture description: " + String(DXGetErrorDescription9(hr)), 
				"D3D9Texture::_createNormTex" );
		}

		if (mFSAAType)
		{
			// create AA surface
			HRESULT hr = d3d9Device->CreateRenderTarget(desc.Width, desc.Height, d3dPF, 
				mFSAAType, 
				mFSAAQuality,
				FALSE, // not lockable
				&textureResources->pFSAASurface, NULL);

			if (FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Unable to create AA render target: " + String(DXGetErrorDescription9(hr)), 
					"D3D9Texture::_createNormTex");
			}

		}

		_setFinalAttributes(d3d9Device, textureResources, 
			desc.Width, desc.Height, 1, D3D9Mappings::_getPF(desc.Format));
		
		// Set best filter type
		if(mMipmapsHardwareGenerated)
		{
			hr = textureResources->pBaseTex->SetAutoGenFilterType(_getBestFilterMethod(d3d9Device));
			if(FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Could not set best autogen filter type: "  + String(DXGetErrorDescription9(hr)), 
					"D3D9Texture::_createNormTex" );
			}
		}

	}
	/****************************************************************************************/
	void D3D9Texture::_createCubeTex(IDirect3DDevice9* d3d9Device)
	{
		// we must have those defined here
		assert(mSrcWidth > 0 || mSrcHeight > 0);

		// determine wich D3D9 pixel format we'll use
		HRESULT hr;
		D3DFORMAT d3dPF = _chooseD3DFormat(d3d9Device);

		// let's D3DX check the corrected pixel format
		hr = D3DXCheckCubeTextureRequirements(d3d9Device, NULL, NULL, 0, &d3dPF, mD3DPool);

		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		DWORD usage = (mUsage & TU_RENDERTARGET) ? D3DUSAGE_RENDERTARGET : 0;
		UINT numMips = (mNumRequestedMipmaps == MIP_UNLIMITED) ? 
			D3DX_DEFAULT : mNumRequestedMipmaps + 1;
		// Check dynamic textures
		if (mUsage & TU_DYNAMIC)
		{
			if (_canUseDynamicTextures(d3d9Device, usage, D3DRTYPE_CUBETEXTURE, d3dPF))
			{
				usage |= D3DUSAGE_DYNAMIC;
				mDynamicTextures = true;
			}
			else
			{
				mDynamicTextures = false;
			}
		}
		// Check sRGB support
		if (mHwGamma)
		{
			mHwGammaReadSupported = _canUseHardwareGammaCorrection(d3d9Device, usage, D3DRTYPE_CUBETEXTURE, d3dPF, false);
			if (mUsage & TU_RENDERTARGET)
				mHwGammaWriteSupported = _canUseHardwareGammaCorrection(d3d9Device, usage, D3DRTYPE_CUBETEXTURE, d3dPF, true);
		}
		// Check FSAA level
		if (mUsage & TU_RENDERTARGET)
		{
			D3D9RenderSystem* rsys = static_cast<D3D9RenderSystem*>(Root::getSingleton().getRenderSystem());
			rsys->determineFSAASettings(d3d9Device, mFSAA, mFSAAHint, d3dPF, false, 
				&mFSAAType, &mFSAAQuality);
		}
		else
		{
			mFSAAType = D3DMULTISAMPLE_NONE;
			mFSAAQuality = 0;
		}

		D3D9Device* device = D3D9RenderSystem::getDeviceManager()->getDeviceFromD3D9Device(d3d9Device);
		const D3DCAPS9& rkCurCaps = device->getD3D9DeviceCaps();			
		
		// check if mip map cube textures are supported
		mMipmapsHardwareGenerated = false;
		if (rkCurCaps.TextureCaps & D3DPTEXTURECAPS_MIPCUBEMAP)
		{
			if (mUsage & TU_AUTOMIPMAP && mNumRequestedMipmaps != 0)
			{
				// use auto.gen. if available
				mMipmapsHardwareGenerated = _canAutoGenMipmaps(d3d9Device, usage, D3DRTYPE_CUBETEXTURE, d3dPF);
				if (mMipmapsHardwareGenerated)
				{
					usage |= D3DUSAGE_AUTOGENMIPMAP;
					numMips = 0;
				}
			}
		}
		else
		{
			// no mip map support for this kind of textures :(
			mNumMipmaps = 0;
			numMips = 1;
		}

		// derive the pool to use
		determinePool();
		TextureResources* textureResources;			

		// Get or create new texture resources structure.
		textureResources = getTextureResources(d3d9Device);
		if (textureResources != NULL)
			freeTextureResources(d3d9Device, textureResources);
		else
			textureResources = allocateTextureResources(d3d9Device);


		// create the texture
		hr = D3DXCreateCubeTexture(	
				d3d9Device,									// device
				static_cast<UINT>(mSrcWidth),		 		// dimension
				numMips,							 		// number of mip map levels
				usage,								 		// usage
				d3dPF,								 		// pixel format
				mD3DPool,
				&textureResources->pCubeTex);		 		// data pointer
		// check result and except if failed
		if (FAILED(hr))
		{
			freeInternalResources();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error creating texture: " + String(DXGetErrorDescription9(hr)), 
				"D3D9Texture::_createCubeTex" );
		}

		// set the base texture we'll use in the render system
		hr = textureResources->pCubeTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&textureResources->pBaseTex);
		if (FAILED(hr))
		{
			freeInternalResources();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't get base texture: " + String(DXGetErrorDescription9(hr)), 
				"D3D9Texture::_createCubeTex" );
		}
		
		// set final tex. attributes from tex. description
		// they may differ from the source image !!!
		D3DSURFACE_DESC desc;
		hr = textureResources->pCubeTex->GetLevelDesc(0, &desc);
		if (FAILED(hr))
		{
			freeInternalResources();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't get texture description: " + String(DXGetErrorDescription9(hr)), 
				"D3D9Texture::_createCubeTex" );
		}

		if (mFSAAType)
		{
			// create AA surface
			HRESULT hr = d3d9Device->CreateRenderTarget(desc.Width, desc.Height, d3dPF, 
				mFSAAType, 
				mFSAAQuality,
				FALSE, // not lockable
				&textureResources->pFSAASurface, NULL);

			if (FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Unable to create AA render target: " + String(DXGetErrorDescription9(hr)), 
					"D3D9Texture::_createCubeTex");
			}
		}

		_setFinalAttributes(d3d9Device, textureResources, 
			desc.Width, desc.Height, 1, D3D9Mappings::_getPF(desc.Format));

		// Set best filter type
		if(mMipmapsHardwareGenerated)
		{
			hr = textureResources->pBaseTex->SetAutoGenFilterType(_getBestFilterMethod(d3d9Device));
			if(FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Could not set best autogen filter type: " + String(DXGetErrorDescription9(hr)), 
					"D3D9Texture::_createCubeTex" );
			}
		}

	}
	/****************************************************************************************/
	void D3D9Texture::_createVolumeTex(IDirect3DDevice9* d3d9Device)
	{
		// we must have those defined here
		assert(mWidth > 0 && mHeight > 0 && mDepth>0);

		if (mUsage & TU_RENDERTARGET)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3D9 Volume texture can not be created as render target !!", 
				"D3D9Texture::_createVolumeTex" );
		}

		// determine which D3D9 pixel format we'll use
		HRESULT hr;
		D3DFORMAT d3dPF = _chooseD3DFormat(d3d9Device);
		// let's D3DX check the corrected pixel format
		hr = D3DXCheckVolumeTextureRequirements(d3d9Device, NULL, NULL, NULL, NULL, 0, &d3dPF, mD3DPool);

		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		DWORD usage = (mUsage & TU_RENDERTARGET) ? D3DUSAGE_RENDERTARGET : 0;
		UINT numMips = (mNumRequestedMipmaps == MIP_UNLIMITED) ? 
			D3DX_DEFAULT : mNumRequestedMipmaps + 1;
		// Check dynamic textures
		if (mUsage & TU_DYNAMIC)
		{
			if (_canUseDynamicTextures(d3d9Device, usage, D3DRTYPE_VOLUMETEXTURE, d3dPF))
			{
				usage |= D3DUSAGE_DYNAMIC;
				mDynamicTextures = true;
			}
			else
			{
				mDynamicTextures = false;
			}
		}
		// Check sRGB support
		if (mHwGamma)
		{
			mHwGammaReadSupported = _canUseHardwareGammaCorrection(d3d9Device, usage, D3DRTYPE_VOLUMETEXTURE, d3dPF, false);
			if (mUsage & TU_RENDERTARGET)
				mHwGammaWriteSupported = _canUseHardwareGammaCorrection(d3d9Device, usage, D3DRTYPE_VOLUMETEXTURE, d3dPF, true);
		}

		D3D9Device* device = D3D9RenderSystem::getDeviceManager()->getDeviceFromD3D9Device(d3d9Device);
		const D3DCAPS9& rkCurCaps = device->getD3D9DeviceCaps();			


		// check if mip map volume textures are supported
		mMipmapsHardwareGenerated = false;
		if (rkCurCaps.TextureCaps & D3DPTEXTURECAPS_MIPVOLUMEMAP)
		{
			if (mUsage & TU_AUTOMIPMAP && mNumRequestedMipmaps != 0)
			{
				// use auto.gen. if available
				mMipmapsHardwareGenerated = _canAutoGenMipmaps(d3d9Device, usage, D3DRTYPE_VOLUMETEXTURE, d3dPF);
				if (mMipmapsHardwareGenerated)
				{
					usage |= D3DUSAGE_AUTOGENMIPMAP;
					numMips = 0;
				}
			}
		}
		else
		{
			// no mip map support for this kind of textures :(
			mNumMipmaps = 0;
			numMips = 1;
		}

		// derive the pool to use
		determinePool();
		TextureResources* textureResources;			

		// Get or create new texture resources structure.
		textureResources = getTextureResources(d3d9Device);
		if (textureResources != NULL)
			freeTextureResources(d3d9Device, textureResources);
		else
			textureResources = allocateTextureResources(d3d9Device);


		// create the texture
		hr = D3DXCreateVolumeTexture(	
				d3d9Device,									// device
				static_cast<UINT>(mWidth),					// dimension
				static_cast<UINT>(mHeight),
				static_cast<UINT>(mDepth),
				numMips,									// number of mip map levels
				usage,										// usage
				d3dPF,										// pixel format
				mD3DPool,
				&textureResources->pVolumeTex);				// data pointer
		// check result and except if failed
		if (FAILED(hr))
		{
			freeInternalResources();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error creating texture: " + String(DXGetErrorDescription9(hr)), 
				"D3D9Texture::_createVolumeTex" );
		}

		// set the base texture we'll use in the render system
		hr = textureResources->pVolumeTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&textureResources->pBaseTex);
		if (FAILED(hr))
		{
			freeInternalResources();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't get base texture: " + String(DXGetErrorDescription9(hr)), 
				"D3D9Texture::_createVolumeTex" );
		}
		
		// set final tex. attributes from tex. description
		// they may differ from the source image !!!
		D3DVOLUME_DESC desc;
		hr = textureResources->pVolumeTex->GetLevelDesc(0, &desc);
		if (FAILED(hr))
		{
			freeInternalResources();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't get texture description: " + String(DXGetErrorDescription9(hr)), 
				"D3D9Texture::_createVolumeTex" );
		}
		_setFinalAttributes(d3d9Device, textureResources,
			desc.Width, desc.Height, desc.Depth, D3D9Mappings::_getPF(desc.Format));
		
		// Set best filter type
		if(mMipmapsHardwareGenerated)
		{
			hr = textureResources->pBaseTex->SetAutoGenFilterType(_getBestFilterMethod(d3d9Device));
			if(FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Could not set best autogen filter type: " + String(DXGetErrorDescription9(hr)), 
					"D3D9Texture::_createCubeTex" );
			}
		}
	}

	/****************************************************************************************/
	void D3D9Texture::_setFinalAttributes(IDirect3DDevice9* d3d9Device, 
		TextureResources* textureResources,
		unsigned long width, unsigned long height, 
        unsigned long depth, PixelFormat format)
	{ 
		// set target texture attributes
		mHeight = height; 
		mWidth = width; 
        mDepth = depth;
		mFormat = format; 

		// Update size (the final size, including temp space because in consumed memory)
		// this is needed in Resource class
		mSize = calculateSize();

		// say to the world what we are doing
		if (mWidth != mSrcWidth ||
			mHeight != mSrcHeight)
		{
			LogManager::getSingleton().logMessage("D3D9 : ***** Dimensions altered by the render system");
			LogManager::getSingleton().logMessage("D3D9 : ***** Source image dimensions : " + StringConverter::toString(mSrcWidth) + "x" + StringConverter::toString(mSrcHeight));
			LogManager::getSingleton().logMessage("D3D9 : ***** Texture dimensions : " + StringConverter::toString(mWidth) + "x" + StringConverter::toString(mHeight));
		}
		
		// Create list of subsurfaces for getBuffer()
		_createSurfaceList(d3d9Device, textureResources);
	}
	/****************************************************************************************/
	void D3D9Texture::_setSrcAttributes(unsigned long width, unsigned long height, 
        unsigned long depth, PixelFormat format)
	{ 
		// set source image attributes
		mSrcWidth = width; 
		mSrcHeight = height; 
		mSrcDepth = depth;
        mSrcFormat = format;
		// say to the world what we are doing
        if (!TextureManager::getSingleton().getVerbose()) return;
		switch (getTextureType())
		{
		case TEX_TYPE_1D:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating 1D RenderTarget, name : '" + getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading 1D Texture, image name : '" + getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_2D:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating 2D RenderTarget, name : '" + getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading 2D Texture, image name : '" + getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_3D:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating 3D RenderTarget, name : '" + getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading 3D Texture, image name : '" + getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_CUBE_MAP:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating Cube map RenderTarget, name : '" + getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading Cube Texture, base image name : '" + getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		default:
			freeInternalResources();
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D9Texture::_setSrcAttributes" );
		}
	}
	/****************************************************************************************/
	D3DTEXTUREFILTERTYPE D3D9Texture::_getBestFilterMethod(IDirect3DDevice9* d3d9Device)
	{
		D3D9Device* device = D3D9RenderSystem::getDeviceManager()->getDeviceFromD3D9Device(d3d9Device);
		const D3DCAPS9& rkCurCaps = device->getD3D9DeviceCaps();			
		
		
		DWORD filterCaps = 0;
		// Minification filter is used for mipmap generation
		// Pick the best one supported for this tex type
		switch (getTextureType())
		{
		case TEX_TYPE_1D:		// Same as 2D
		case TEX_TYPE_2D:		filterCaps = rkCurCaps.TextureFilterCaps;	break;
		case TEX_TYPE_3D:		filterCaps = rkCurCaps.VolumeTextureFilterCaps;	break;
		case TEX_TYPE_CUBE_MAP:	filterCaps = rkCurCaps.CubeTextureFilterCaps;	break;
		}
		if(filterCaps & D3DPTFILTERCAPS_MINFGAUSSIANQUAD)
			return D3DTEXF_GAUSSIANQUAD;
		
		if(filterCaps & D3DPTFILTERCAPS_MINFPYRAMIDALQUAD)
			return D3DTEXF_PYRAMIDALQUAD;
		
		if(filterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
			return D3DTEXF_ANISOTROPIC;
		
		if(filterCaps & D3DPTFILTERCAPS_MINFLINEAR)
			return D3DTEXF_LINEAR;
		
		if(filterCaps & D3DPTFILTERCAPS_MINFPOINT)
			return D3DTEXF_POINT;
		
		return D3DTEXF_POINT;
	}
	/****************************************************************************************/
	bool D3D9Texture::_canUseDynamicTextures(IDirect3DDevice9* d3d9Device, 
		DWORD srcUsage, 
		D3DRESOURCETYPE srcType, 
		D3DFORMAT srcFormat)
	{		
		HRESULT hr;
		IDirect3D9* pD3D = NULL;

		hr = d3d9Device->GetDirect3D(&pD3D);
		if (FAILED(hr))
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
				"GetDirect3D failed !!!", 				
				"D3D9Texture::_canUseDynamicTextures" );
		}
		if (pD3D != NULL)
			pD3D->Release();
	
		D3D9Device* device = D3D9RenderSystem::getDeviceManager()->getDeviceFromD3D9Device(d3d9Device);
		const D3DCAPS9& rkCurCaps = device->getD3D9DeviceCaps();						
		D3DFORMAT eBackBufferFormat = device->getBackBufferFormat();

		
		// Check for dynamic texture support
		
		// check for auto gen. mip maps support
		hr = pD3D->CheckDeviceFormat(
			rkCurCaps.AdapterOrdinal, 
			rkCurCaps.DeviceType, 
			eBackBufferFormat, 
			srcUsage | D3DUSAGE_DYNAMIC,
			srcType,
			srcFormat);

		if (hr == D3D_OK)
			return true;
		else
			return false;
	}
	/****************************************************************************************/
	bool D3D9Texture::_canUseHardwareGammaCorrection(IDirect3DDevice9* d3d9Device,
		DWORD srcUsage, 
		D3DRESOURCETYPE srcType, D3DFORMAT srcFormat, bool forwriting)
	{
		HRESULT hr;
		IDirect3D9* pD3D = NULL;

		hr = d3d9Device->GetDirect3D(&pD3D);
		if (FAILED(hr))
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
				"GetDirect3D failed !!!", 
				"D3D9Texture::_canUseDynamicTextures" );
		}
		if (pD3D != NULL)
			pD3D->Release();

		D3D9Device* device = D3D9RenderSystem::getDeviceManager()->getDeviceFromD3D9Device(d3d9Device);
		const D3DCAPS9& rkCurCaps = device->getD3D9DeviceCaps();						
		D3DFORMAT eBackBufferFormat = device->getBackBufferFormat();


		// Always check 'read' capability here
		// We will check 'write' capability only in the context of a render target
		if (forwriting)
			srcUsage |= D3DUSAGE_QUERY_SRGBWRITE;
		else
			srcUsage |= D3DUSAGE_QUERY_SRGBREAD;

		// Check for sRGB support		
		// check for auto gen. mip maps support
		hr = pD3D->CheckDeviceFormat(
			rkCurCaps.AdapterOrdinal, 
			rkCurCaps.DeviceType, 
			eBackBufferFormat, 
			srcUsage,
			srcType,
			srcFormat);
		if (hr == D3D_OK)
			return true;
		else
			return false;

	}
	/****************************************************************************************/
	bool D3D9Texture::_canAutoGenMipmaps(IDirect3DDevice9* d3d9Device, 
		DWORD srcUsage, D3DRESOURCETYPE srcType, D3DFORMAT srcFormat)
	{
		HRESULT hr;
		IDirect3D9* pD3D = NULL;

		hr = d3d9Device->GetDirect3D(&pD3D);
		if (FAILED(hr))
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
				"GetDirect3D failed !!!", 
				"D3D9Texture::_canUseDynamicTextures" );
		}
		if (pD3D != NULL)
			pD3D->Release();

		D3D9Device* device = D3D9RenderSystem::getDeviceManager()->getDeviceFromD3D9Device(d3d9Device);
		const D3DCAPS9& rkCurCaps = device->getD3D9DeviceCaps();						
		D3DFORMAT eBackBufferFormat = device->getBackBufferFormat();


		// Hacky override - many (all?) cards seem to not be able to autogen on 
		// textures which are not a power of two
		// Can we even mipmap on 3D textures? Well
		if ((mWidth & mWidth-1) || (mHeight & mHeight-1) || (mDepth & mDepth-1))
			return false;

		if (rkCurCaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP)
		{
			HRESULT hr;
			// check for auto gen. mip maps support
			hr = pD3D->CheckDeviceFormat(
					rkCurCaps.AdapterOrdinal, 
					rkCurCaps.DeviceType, 
					eBackBufferFormat, 
					srcUsage | D3DUSAGE_AUTOGENMIPMAP,
					srcType,
					srcFormat);
			// this HR could a SUCCES
			// but mip maps will not be generated
			if (hr == D3D_OK)
				return true;
			else
				return false;
		}
		else
			return false;
	}
	/****************************************************************************************/
	D3DFORMAT D3D9Texture::_chooseD3DFormat(IDirect3DDevice9* d3d9Device)
	{		
		// Choose frame buffer pixel format in case PF_UNKNOWN was requested
		if(mFormat == PF_UNKNOWN)
		{	
			D3D9Device* device = D3D9RenderSystem::getDeviceManager()->getDeviceFromD3D9Device(d3d9Device);
			
			return device->getBackBufferFormat();		
		}

		// Choose closest supported D3D format as a D3D format
		return D3D9Mappings::_getPF(D3D9Mappings::_getClosestSupportedPF(mFormat));
	}
	/****************************************************************************************/
	// Macro to hide ugly cast
	#define GETLEVEL(face,mip) \
	 	static_cast<D3D9HardwarePixelBuffer*>(mSurfaceList[face*(mNumMipmaps+1)+mip].get())
	void D3D9Texture::_createSurfaceList(IDirect3DDevice9* d3d9Device, TextureResources* textureResources)
	{
		IDirect3DSurface9 *surface;
		IDirect3DVolume9 *volume;
		D3D9HardwarePixelBuffer *buffer;				
		size_t mip, face;

		
		assert(textureResources != NULL);
		assert(textureResources->pBaseTex);
		// Make sure number of mips is right
		mNumMipmaps = textureResources->pBaseTex->GetLevelCount() - 1;
		// Need to know static / dynamic
		unsigned int bufusage;
		if ((mUsage & TU_DYNAMIC) && mDynamicTextures)
		{
			bufusage = HardwareBuffer::HBU_DYNAMIC;
		}
		else
		{
			bufusage = HardwareBuffer::HBU_STATIC;
		}
		if (mUsage & TU_RENDERTARGET)
		{
			bufusage |= TU_RENDERTARGET;
		}
		
		uint surfaceCount  = static_cast<uint>((getNumFaces() * (mNumMipmaps + 1)));
		bool updateOldList = mSurfaceList.size() == surfaceCount;
		if(!updateOldList)
		{			
			// Create new list of surfaces	
			mSurfaceList.clear();
			for(size_t face=0; face<getNumFaces(); ++face)
			{
				for(size_t mip=0; mip<=mNumMipmaps; ++mip)
				{
					buffer = new D3D9HardwarePixelBuffer((HardwareBuffer::Usage)bufusage, this);
					mSurfaceList.push_back(HardwarePixelBufferSharedPtr(buffer));
				}
			}
		}

		

		switch(getTextureType()) {
		case TEX_TYPE_2D:
		case TEX_TYPE_1D:
			assert(textureResources->pNormTex);
			// For all mipmaps, store surfaces as HardwarePixelBufferSharedPtr
			for(mip=0; mip<=mNumMipmaps; ++mip)
			{
				if(textureResources->pNormTex->GetSurfaceLevel(static_cast<UINT>(mip), &surface) != D3D_OK)
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Get surface level failed",
		 				"D3D9Texture::_createSurfaceList");

				D3D9HardwarePixelBuffer* currPixelBuffer = GETLEVEL(0, mip);
				
				currPixelBuffer->bind(d3d9Device, surface, textureResources->pFSAASurface,
					mHwGammaWriteSupported, mFSAA, mName, textureResources->pBaseTex);

				if (mip == 0 && mNumRequestedMipmaps != 0 && (mUsage & TU_AUTOMIPMAP))
					currPixelBuffer->_setMipmapping(true, mMipmapsHardwareGenerated);

				// decrement reference count, the GetSurfaceLevel call increments this
				// this is safe because the pixel buffer keeps a reference as well
				surface->Release();			
			}
			
			break;
		case TEX_TYPE_CUBE_MAP:
			assert(textureResources->pCubeTex);
			// For all faces and mipmaps, store surfaces as HardwarePixelBufferSharedPtr
			for(face=0; face<6; ++face)
			{
				for(mip=0; mip<=mNumMipmaps; ++mip)
				{
					if(textureResources->pCubeTex->GetCubeMapSurface((D3DCUBEMAP_FACES)face, static_cast<UINT>(mip), &surface) != D3D_OK)
						OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Get cubemap surface failed",
		 				"D3D9Texture::getBuffer");

					D3D9HardwarePixelBuffer* currPixelBuffer = GETLEVEL(face, mip);
					
					currPixelBuffer->bind(d3d9Device, surface, textureResources->pFSAASurface,
						mHwGammaWriteSupported, mFSAA, mName, textureResources->pBaseTex);

					if (mip == 0 && mNumRequestedMipmaps != 0 && (mUsage & TU_AUTOMIPMAP))
						currPixelBuffer->_setMipmapping(true, mMipmapsHardwareGenerated);


					// decrement reference count, the GetSurfaceLevel call increments this
					// this is safe because the pixel buffer keeps a reference as well
					surface->Release();				
				}				
			}
			break;
		case TEX_TYPE_3D:
			assert(textureResources->pVolumeTex);
			// For all mipmaps, store surfaces as HardwarePixelBufferSharedPtr
			for(mip=0; mip<=mNumMipmaps; ++mip)
			{
				if(textureResources->pVolumeTex->GetVolumeLevel(static_cast<UINT>(mip), &volume) != D3D_OK)
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Get volume level failed",
		 				"D3D9Texture::getBuffer");	
						
				D3D9HardwarePixelBuffer* currPixelBuffer = GETLEVEL(0, mip);

				currPixelBuffer->bind(d3d9Device, volume, textureResources->pBaseTex);

				if (mip == 0 && mNumRequestedMipmaps != 0 && (mUsage & TU_AUTOMIPMAP))
					currPixelBuffer->_setMipmapping(true, mMipmapsHardwareGenerated);

				// decrement reference count, the GetSurfaceLevel call increments this
				// this is safe because the pixel buffer keeps a reference as well
				volume->Release();
			}
			break;
		};					
	}
	#undef GETLEVEL
	/****************************************************************************************/
	HardwarePixelBufferSharedPtr D3D9Texture::getBuffer(size_t face, size_t mipmap) 
	{
		if(face >= getNumFaces())
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "A three dimensional cube has six faces",
					"D3D9Texture::getBuffer");
		if(mipmap > mNumMipmaps)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Mipmap index out of range",
					"D3D9Texture::getBuffer");
		size_t idx = face*(mNumMipmaps+1) + mipmap;

		IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getActiveD3D9Device();
		TextureResources* textureResources = getTextureResources(d3d9Device);
		if (textureResources == NULL || textureResources->pBaseTex == NULL)
		{				
			createTextureResources(d3d9Device);
			textureResources = getTextureResources(d3d9Device);			
		}
	
		assert(textureResources != NULL);
		assert(idx < mSurfaceList.size());
		return mSurfaceList[idx];
	}
	//---------------------------------------------------------------------
	bool D3D9Texture::useDefaultPool()
	{
		// Determine D3D pool to use
		// Use managed unless we're a render target or user has asked for 
		// a dynamic texture, and device supports D3DUSAGE_DYNAMIC (because default pool
		// resources without the dynamic flag are not lockable)
		return (mUsage & TU_RENDERTARGET) || ((mUsage & TU_DYNAMIC) && mDynamicTextures);
	}
	
	//---------------------------------------------------------------------
	void D3D9Texture::notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device) 
	{	
		if (D3D9RenderSystem::getResourceManager()->getCreationPolicy() == RCP_CREATE_ON_ALL_DEVICES)
			createTextureResources(d3d9Device);
	}

	//---------------------------------------------------------------------
	void D3D9Texture::notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device) 
	{						
		DeviceToTextureResourcesIterator it = mMapDeviceToTextureResources.find(d3d9Device);

		if (it != mMapDeviceToTextureResources.end())
		{			
			std::stringstream ss;

			ss << "D3D9 device: 0x[" << d3d9Device << "] destroy. Releasing D3D9 texture: " << mName;
			LogManager::getSingleton().logMessage(ss.str());

			TextureResources* textureResource = it->second;

			// Just free any internal resources, don't call unload() here
			// because we want the un-touched resource to keep its unloaded status
			// after device reset.
			freeTextureResources(d3d9Device, textureResource);

			SAFE_DELETE(textureResource);

			mMapDeviceToTextureResources.erase(it);

			LogManager::getSingleton().logMessage("Released D3D9 texture: " + mName);	
		}	
	}

	//---------------------------------------------------------------------
	void D3D9Texture::notifyOnDeviceLost(IDirect3DDevice9* d3d9Device) 
	{
		if(mD3DPool == D3DPOOL_DEFAULT)
		{
			DeviceToTextureResourcesIterator it = mMapDeviceToTextureResources.find(d3d9Device);

			if (it != mMapDeviceToTextureResources.end())
			{
				std::stringstream ss;

				ss << "D3D9 device: 0x[" << d3d9Device << "] lost. Releasing D3D9 texture: " << mName;
				LogManager::getSingleton().logMessage(ss.str());

				TextureResources* textureResource = it->second;

				// Just free any internal resources, don't call unload() here
				// because we want the un-touched resource to keep its unloaded status
				// after device reset.
				freeTextureResources(d3d9Device, textureResource);
				
				LogManager::getSingleton().logMessage("Released D3D9 texture: " + mName);	
			}					
		}		
	}

	//---------------------------------------------------------------------
	void D3D9Texture::notifyOnDeviceReset(IDirect3DDevice9* d3d9Device) 
	{		
		if(mD3DPool == D3DPOOL_DEFAULT)
		{
			reloadTexture();
		}
	}

	//---------------------------------------------------------------------
	void D3D9Texture::reloadTexture()
	{
		LogManager::getSingleton().logMessage(
			"Recreating D3D9 default pool texture: " + mName);
		// We just want to create the texture resources if:
		// 1. This is a render texture, or
		// 2. This is a manual texture with no loader, or
		// 3. This was an unloaded regular texture (preserve unloaded state)
		if ((mIsManual && !mLoader) || (mUsage & TU_RENDERTARGET) || !isLoaded())
		{
			// just recreate any internal resources
			createInternalResources();
		}
		// Otherwise, this is a regular loaded texture, or a manual texture with a loader
		else
		{
			// The internal resources already freed, need unload/load here:
			// 1. Make sure resource memory usage statistic correction.
			// 2. Don't call unload() in releaseIfDefaultPool() because we want
			//    the un-touched resource keep unload status after device reset.
			unload();
			// if manual, we need to recreate internal resources since load() won't do that
			if (mIsManual)
				createInternalResources();
			load();
		}
		LogManager::getSingleton().logMessage(
			"Recreated D3D9 default pool texture: " + mName);
	}

	//---------------------------------------------------------------------
	IDirect3DBaseTexture9* D3D9Texture::getTexture()
	{
		TextureResources* textureResources;			
		IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getActiveD3D9Device();
			
		textureResources = getTextureResources(d3d9Device);		
		if (textureResources == NULL || textureResources->pBaseTex == NULL)
		{			
			createTextureResources(d3d9Device);
			textureResources = getTextureResources(d3d9Device);			
		}
		assert(textureResources); 
		assert(textureResources->pBaseTex); 

		return textureResources->pBaseTex;
	}

	//---------------------------------------------------------------------
	IDirect3DTexture9* D3D9Texture::getNormTexture()
	{
		TextureResources* textureResources;
		IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getActiveD3D9Device();
		
		textureResources = getTextureResources(d3d9Device);		
		if (textureResources == NULL || textureResources->pNormTex == NULL)
		{
			createTextureResources(d3d9Device);
			textureResources = getTextureResources(d3d9Device);			
		}
		assert(textureResources); 
		assert(textureResources->pNormTex); 

		return textureResources->pNormTex;
	}

	//---------------------------------------------------------------------
	IDirect3DCubeTexture9* D3D9Texture::getCubeTexture()
	{
		TextureResources* textureResources;
		IDirect3DDevice9* d3d9Device = D3D9RenderSystem::getActiveD3D9Device();
		
		textureResources = getTextureResources(d3d9Device);		
		if (textureResources == NULL || textureResources->pCubeTex)
		{
			createTextureResources(d3d9Device);
			textureResources = getTextureResources(d3d9Device);			
		}
		assert(textureResources); 
		assert(textureResources->pCubeTex); 

		return textureResources->pCubeTex;
	}	

	/****************************************************************************************/
	D3D9RenderTexture::D3D9RenderTexture(const String &name, 
		D3D9HardwarePixelBuffer *buffer, 
		bool writeGamma, 
		uint fsaa) : RenderTexture(buffer, 0)
	{
		mName = name;
		mHwGamma = writeGamma;
		mFSAA = fsaa;		
	}
	//---------------------------------------------------------------------
    void D3D9RenderTexture::update(bool swap)
    {
		D3D9DeviceManager* deviceManager = D3D9RenderSystem::getDeviceManager();       	
		D3D9Device* currRenderWindowDevice = deviceManager->getActiveRenderTargetDevice();

		if (currRenderWindowDevice != NULL)
		{
			if (currRenderWindowDevice->isDeviceLost() == false)
				RenderTexture::update(swap);
		}
		else
		{
			for (UINT i=0; i < deviceManager->getDeviceCount(); ++i)
			{
				D3D9Device* device = deviceManager->getDevice(i);

				if (device->isDeviceLost() == false)
				{
					deviceManager->setActiveRenderTargetDevice(device);
					RenderTexture::update(swap);
					deviceManager->setActiveRenderTargetDevice(NULL);
				}								
			}
		}
	}
	//---------------------------------------------------------------------
	void D3D9RenderTexture::getCustomAttribute( const String& name, void *pData )
	{
		if(name == "DDBACKBUFFER")
		{
			if (mFSAA > 0)
			{
				// rendering to AA surface
				IDirect3DSurface9 ** pSurf = (IDirect3DSurface9 **)pData;
				*pSurf = static_cast<D3D9HardwarePixelBuffer*>(mBuffer)->getFSAASurface(D3D9RenderSystem::getActiveD3D9Device());
				return;
			}
			else
			{
				IDirect3DSurface9 ** pSurf = (IDirect3DSurface9 **)pData;
				*pSurf = static_cast<D3D9HardwarePixelBuffer*>(mBuffer)->getSurface(D3D9RenderSystem::getActiveD3D9Device());
				return;
			}
		}
		else if(name == "HWND")
		{
			HWND *pHwnd = (HWND*)pData;
			*pHwnd = NULL;
			return;
		}
		else if(name == "BUFFER")
		{
			*static_cast<HardwarePixelBuffer**>(pData) = mBuffer;
			return;
		}
	}    
	//---------------------------------------------------------------------
	void D3D9RenderTexture::swapBuffers(bool waitForVSync /* = true */)
	{
		// Only needed if we have to blit from AA surface
		if (mFSAA > 0)
		{
			D3D9DeviceManager* deviceManager = D3D9RenderSystem::getDeviceManager();     					
			D3D9HardwarePixelBuffer* buf = static_cast<D3D9HardwarePixelBuffer*>(mBuffer);

			for (UINT i=0; i < deviceManager->getDeviceCount(); ++i)
			{
				D3D9Device* device = deviceManager->getDevice(i);
				 				
				if (device->isDeviceLost() == false)
				{
					IDirect3DDevice9* d3d9Device = device->getD3D9Device();

					HRESULT hr = d3d9Device->StretchRect(buf->getFSAASurface(d3d9Device), 0, 
						buf->getSurface(d3d9Device), 0, D3DTEXF_NONE);

					if (FAILED(hr))
					{
						OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
							"Unable to copy AA buffer to final buffer: " + String(DXGetErrorDescription9(hr)), 
							"D3D9RenderTexture::swapBuffers");
					}
				}								
			}																		
		}			
	}	
}
