import sys
import os
import struct
import array 
import numpy as np
import scipy.misc
import matplotlib.pyplot as plt
import matplotlib.colors as colors
import matplotlib.pylab as plab

class MRCImage:

	field_labels =  (('nx', 'i'), 
			('ny','i'), 
			('nz','i'), 
			('mode','i'), 
			('nxStart','i'), 
			('nyStart','i'), 
			('nzStart','i'), 
			('mx','i'), 
			('my','i'), 
			('mz','i'), 
			('a','f'), 
			('b','f'), 
			('c','f'),
			('alpha','f'),
			('beta','f'),
			('gamma','f'),
			('mapc','i'),
			('mapr','i'),
			('maps','i'),
			('amin','f'),
			('amax','f'),
			('amean','f'),
			('ispg','i'),
			('nsymbt','i'),
			('extra','P'),
			('xOrigin','f'),
			('yOrigin','f'),
			('zOrigin','f'),
			('map','4s'),
			('machst','4s'),
			('rms','f'),
			('nlabl','i'),
			('labels','P'),
			)

	def __init__(self, f):
		filename = os.path.basename(f.name)
		self.name = os.path.splitext(filename)[0]
		self.readHeader(f)
		self.readImage(f)
	
	def readHeader(self, f):
		for label in self.field_labels:
			fmt = '='+label[1]
			if label[1] != 'P':
				bytes_read = f.read(4)
				if not bytes_read: 
					break
				field = struct.unpack(fmt, bytes_read)[0]
				print(label[0]+' = '+str(field))
				self.__dict__.update({label[0]:field})
			else:
				if label[0] == 'extra':
					extra = []
					fmt = '=f'
					for i in range(25):
						bytes_read = f.read(4)
						if not bytes_read: 
							break
						field = struct.unpack(fmt, bytes_read)[0]
						extra.append(field)
					self.__dict__.update({'extra':extra})
				elif label[0] == 'labels':
					labels = []
					fmt = '80s'
					for i in range(10):
						bytes_read = f.read(80)
						if not bytes_read: 
							break
						field = struct.unpack(fmt, bytes_read)[0]
						labels.append(field)
					self.__dict__.update({'lalbels':labels})
	
	def readImage(self, f):
		# check if the mode was read by the header
		if 'mode' in self.__dict__:
			if self.mode == 0:
				print('MRC image mode: '+str(self.mode))
				fmt = np.uint8
				#self.image = array.array('b')
			elif self.mode == 1:
				print('MRC image mode: '+str(self.mode))
				fmt = np.float16
			elif self.mode == 2:
				print('MRC image mode: '+str(self.mode))
				fmt = np.float32
				#self.image = array.array('f')
				#self.image.fromfile(f,self.nx*self.ny*self.nz)
			else:
				raise Exception("Unsupported MRC image mode: "+self.mode)
			
			self.image = np.fromfile(f,fmt,self.nx*self.ny*self.nz)
			if(self.nz == 1):
				self.image = self.image.reshape(self.ny,self.nx)
			else:
				self.image = self.image.reshape(self.ny, self.nx, self.nz)
				print('Warning: Image dimensions: '+str(self.image.shape))
		else:
			sys.exit('Error: The mode in the image header was not read!')
	
def scaleImages(mrc_image_list):
        m = max([ np.absolute(mrcImage.image).max() for mrcImage in mrc_image_list])
	print('Max Intensity in the images is: '+str(m))
        for mrcImage in mrc_image_list:
            mrcImage.image/m
	return m 

def cropImage(mrcImage, width, height):
    "crops the mrc image to the width and height"
    image = mrcImage.image
    [orig_height, orig_width] = np.shape(image)
    if width == orig_width and height == orig_height:
        return mrcImage
    elif width > orig_width and height > orig_height:
        exit("the image cannnot be cropped to an size larger then itself")
    else:
        d_width = orig_width - width
        d_height = orig_height - height
        d2_width = d_width/2
        d2_height = d_height/2
        if d2_width > 0:
            if d_width % 2 > 0:
                istart = d2_width
            else:
                istart = d2_width-1
            iend = -d2_width-1
            image = image[:,istart:iend]
        if d2_height > 0:
            if d_height % 2 > 0:
                istart = d2_height
            else:
                istart = d2_height-1
            iend = -d2_height-1
            image = image[istart:iend,:]
        mrcImage.image = image
        mrcImage.nx = width
        mrcImage.ny = height
        return mrcImage





def cropImages(mrcImage1, mrcImage2):
        """if the images do not habve the same size they are cropped to the smaller
        width and height"""
        width1 = mrcImage1.nx
        height1 = mrcImage1.ny
        width2 = mrcImage2.nx
        height2 = mrcImage2.ny
        if width1 == width2 and height1 == height2:
            #no cropping needed
            return (width1, height1)
        [width_min, width_max] = minmax(width1, width2) 
        [height_min, height_max] = minmax(height1, height2)
        d_width = width_max - width_min
        d2_width = d_width/2
        d_height = height_max - height_min
        d2_height = d_height/2
        if d2_width > 0:
            if d_width % 2 > 0:
                istart = d2_width
            else:
                istart = d2_width-1
            iend = -d2_width-1
            if width1 > width_min:
                mrcImage1.image = mrcImage1.image[:,istart:iend]
                #mrcImage1.image = mrcImage1.image[istart:iend,:]
                mrcImage1.nx = width_min
            else:
                mrcImage2.image = mrcImage2.image[:,istart:iend]
                mrcImage2.nx = width_min
        if d2_height > 0:
            if d_height % 2 > 0:
                istart = d2_height
            else:
                istart = d2_height-1
            iend = -d2_height-1
            if height1 > height_min:
                mrcImage1.image = mrcImage1.image[istart:iend,:]
                mrcImage1.ny = height_min
            else:
                mrcImage2.image = mrcImage2.image[istart:iend,:]
                mrcImage2.ny = height_min
        print(": image 1 cropped to size:"+str(np.shape(mrcImage1.image)))
        print(": image 2 cropped to size:"+str(np.shape(mrcImage2.image)))
        return (width_min, height_min)

def minmax(value1, value2):
    """compares the values and returns the minimum and maximum"""
    if value1 < value2:
        return [value1, value2]
    else:
        return [value2, value1]

def cutImages(mrc_image_list):
    for mrc_image in mrc_image_list:
        im = mrc_image.image
        im[im<0] = 0.0
	mrc_image.image = im

def saveImage(mrcImage):
	image = mrcImage.image
	title = mrcImage.name
	filename = title+'.pdf'
	plt.imsave(filename, image,  vmin=None, vmax=None, cmap=plt.cm.jet, format='pdf', origin='lower')
 	return image

def getDiffmap(mrc_image1, mrc_image2):
    """returns the difference map of the two images"""
    im1 = mrc_image1.image  
    im2 = mrc_image2.image
    diffmap = im1-im2
    return diffmap


def significantDifferences(mrc_image1, mrc_image2, mrc_mixed1, mrc_mixed2):
    """determines which differences are significant base on the mixed merge data set """
    im1 = mrc_image1.image  
    im2 = mrc_image2.image  
    mix1 = mrc_mixed1.image  
    mix2 = mrc_mixed2.image
    diffmap = im1-im2
    variance = mix1-mix2
    diffmap[abs(diffmap)<abs(variance)] = 0.0
    return diffmap

def plotImage(image, crange=0.0,title=""):
        plt.figure()
	plt.title(title)
        if crange > 0.0:
	    norm = colors.Normalize(vmin=0.0, vmax=crange) 
	    plt.imshow(image, norm=norm)
        else:
	    plt.imshow(image)
        plt.colorbar()

def plotMRCImage(mrcImage,crange=0.0):
	image = mrcImage.image
	plotImage(image, crange, mrcImage.name) 	
        return image


def plotDiffmap(contour, diffmap, mapName1="map1", mapName2="map2", colormap="jet"):
	max_range = max(diffmap.max(),abs(diffmap.min()))
	norm = colors.Normalize(vmin=-max_range, vmax=max_range) 
	print('max_range is '+str(max_range))
	title = mapName1+' - '+mapName2
	filename = title+'.pdf'
	fig = plt.figure()
	plt.title(title)
	plt.hold(True)
	plt.imshow(diffmap, origin='upper', norm=norm)
        #if(colormap == "rwb")
        plt.set_cmap(colormap)
	plt.colorbar()
	plt.contour(contour, [0])
	#plt.contour(image2, [0], colors='b')
	plt.hold(False)
	plt.savefig(filename)
        plt.show()
	return diffmap
	