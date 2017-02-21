# Python utilities for Focus
# Author: Ricardo Righetto
# E-mail: ricardo.righetto@unibas.ch

import numpy as np
try:
	# SciPy FFT pack is faster than NumPy's:
	import scipy.fftpack as fft

except ImportError:

	import numpy.fft as fft

def RadialIndices( imsize = [100, 100], rounding=True, normalize=False ):
# Returns radius and angles for each pixel (or voxel) in a 2D image or 3D volume of shape = imsize
# For 2D returns the angle with the horizontal x- axis
# For 3D returns the angle with the horizontal x,y plane
# If imsize is a scalar, will default to 2D.
# Rounding is to ensure "perfect" radial symmetry, desirable in most applications.
# Normalize=True will normalize the radius to values between 0.0 and 1.0 (in relation to the object's shortest dimension).
# Note: the radii values returned by this function are consistent with the sampling frequency in SciPy/NumPy 

	if np.isscalar(imsize):

		imsize = [imsize, imsize]

	if len( imsize ) > 3:

		raise ValueError ( "Object should not have dimensions larger than 3: len(imsize) = %d " % len(imsize))

	import warnings
	with warnings.catch_warnings():
		warnings.filterwarnings( "ignore", category=RuntimeWarning )

		# d = np.ones( len( imsize ) )

		# if not normalize:

		# 	for i in np.arange( len( imsize ) )

		# 		d[i] = 1./imsize[i]

		m = np.mod(imsize, 2) # Check if dimensions are odd or even

		if len(imsize) == 2:

			# [xmesh, ymesh] = np.mgrid[-imsize[0]/2+1:imsize[0]/2+1, -imsize[1]/2+1:imsize[1]/2+1].astype(np.float)
			# The definition below is consistent with scipy/numpy fft.fftfreq:
			[xmesh, ymesh] = np.mgrid[-imsize[0]//2+m[0]:(imsize[0]-1)//2+1, -imsize[1]//2+m[1]:(imsize[1]-1)//2+1].astype(np.float)

			rmesh = np.sqrt( xmesh*xmesh + ymesh*ymesh )
			
			amesh = np.arctan( ymesh / xmesh )

		else:

			# [xmesh, ymesh, zmesh] = np.mgrid[-imsize[0]/2:imsize[0]/2, -imsize[1]/2:imsize[1]/2, -imsize[2]/2:imsize[2]/2].astype(np.float)
			# The definition below is consistent with scipy/numpy fft.fftfreq:
			[xmesh, ymesh, zmesh] = np.mgrid[-imsize[0]//2+m[0]:(imsize[0]-1)//2+1, -imsize[1]//2+m[1]:(imsize[1]-1)//2+1, -imsize[2]//2+m[2]:(imsize[2]-1)//2+1].astype(np.float)

			rmesh = np.sqrt( xmesh*xmesh + ymesh*ymesh + zmesh*zmesh )

			amesh = np.arccos( zmesh / rmesh )


	if rounding:

		rmesh = np.round( rmesh )

	if normalize:

		rmesh /= np.min( imsize )

	else:

		rmesh = rmesh.astype(np.int)

	return rmesh, np.nan_to_num( amesh )

def RotationalAverage( img ):
# Compute the rotational average of a 2D image or 3D volume

	rmesh = RadialIndices( img.shape, rounding=True )[0]

	rotavg = np.zeros( img.shape )

	for r in np.unique( rmesh ):

		idx = rmesh == r
		rotavg[idx] = img[idx].mean()

	return rotavg

def SoftMask( imsize = [100, 100], radius = 0.5, width = 6.0 ):
# Generates a circular or spherical mask with a soft cosine edge

	if np.isscalar(imsize):

		imsize = [imsize, imsize]

	if len(imsize) > 3:

		raise ValueError ( "Object should not have dimensions larger than 3: len(imsize) = %d " % len(imsize))

	if width < 0.0:

		width = 0.0

	if width > 0.0 and width <= 1.0:

		width = 0.5 * width * np.min( imsize )

	if radius < 0.0 or np.any( imsize < radius):

		radius = np.min( imsize ) - float(width)/2

	if radius > 0.0 and radius <= 1.0:

		radius = radius * np.min( imsize )

	width *= 0.5
	radius *= 0.5

	rii = radius + width/2
	rih = radius - width/2

	rmesh = RadialIndices( imsize, rounding=True )[0]

	mask = np.zeros( imsize )

	fill_idx = rmesh < rih
	mask[fill_idx] = 1.0

	rih_idx = rmesh >= rih
	rii_idx = rmesh <= rii
	edge_idx = rih_idx * rii_idx

	mask[edge_idx] = ( 1.0 + np.cos( np.pi * ( rmesh[edge_idx] - rih ) / width ) ) / 2.0

	return mask

def FilterGauss( img, apix=1.0, lp=-1, hp=-1, return_filter=False ):
# Gaussian band-pass filtering of images.

	rmesh = RadialIndices( img.shape, normalize=True )[0] / apix
	rmesh2 = rmesh*rmesh

	if lp <= 0.0:

		lowpass = 1.0

	else:

		lowpass = np.exp( - lp ** 2 * rmesh2 / 2 )

	if hp <= 0.0:

		highpass = 1.0

	else:

		highpass = 1.0 - np.exp( - hp ** 2 * rmesh2 / 2 )

	bandpass = lowpass * highpass

	ft = fft.fftshift( fft.fftn( img ) )

	filtered = fft.ifftn( fft.ifftshift( ft * bandpass ) )

	if return_filter:

		return filtered.real, bandpass

	else:

		return filtered.real

def FilterBfactor( img, apix=1.0, B=0.0, return_filter=False ):
# Applies a B-factor to images. B can be positive or negative.

	rmesh = RadialIndices( img.shape, normalize=True )[0] / apix
	rmesh2 = rmesh*rmesh

	bfac = np.exp( - (B * rmesh2  ) /  4  )

	ft = fft.fftshift( fft.fftn( img ) )

	filtered = fft.ifftn( fft.ifftshift( ft * bfac ) )

	if return_filter:

		return filtered.real, bfac

	else:

		return filtered.real

def FilterCosine( img, apix=1.0, lp=-1, hp=-1, width=6.0, return_filter=False ):
# Band-pass filtering of images with a cosine edge. Good to approximate a top-hat filter while still reducing edge artifacts.

	if width < 0.0:

		width = 0.0

	if width > 0.0 and width <= 1.0:

		width = 0.5 * width * np.min( img.shape )

	if lp <= 0.0:

		lowpass = 1.0

	else:

		lowpass = SoftMask( img.shape, radius=np.min( img.shape ) * apix/lp, width=width )

	if hp <= 0.0:

		highpass = 1.0

	else:

		highpass = 1.0 - SoftMask( img.shape, radius=np.min( img.shape ) * apix/hp, width=width )

	bandpass = lowpass * highpass

	ft = fft.fftshift( fft.fftn( img ) )

	filtered = fft.ifftn( fft.ifftshift( ft * bandpass ) )

	if return_filter:

		return filtered.real, bandpass

	else:

		return filtered.real

def Resample( img, newsize=None, apix=1.0, newapix=None ):
# Resizes a real image or volume by cropping/padding its Fourier Transform, i.e. resampling.

	if newsize == None and newapix == None:

		newsize = img.shape

	elif newapix != None:

		newsize = np.round( np.array( img.shape ) * apix / newapix ).astype( 'int' )

	return fft.irfftn( fft.rfftn( img ), s = newsize ).real

def NormalizeImg( img, mean=0.0, std=1.0 ):
# Normalizes an image to specified mean and standard deviation:

	return (img - img.mean() + mean) * std / img.std()

# def Resize( img, newsize=None, padval=None ):
# # Resizes a real image or volume by cropping/padding. I.e. sampling is not changed.

##### UNDER CONSTRUCTION #####

#     if newsize == None:

#         return img

#     else:

#         imgshape = np.array( img.shape )
#         newsize = np.array( newsize )

#         maxdim = np.maximum( imgshape, newsize )

#         imgones = np.ones( imgshape ).astype('int')
#         newones = np.ones( newsize ).astype('int')
#         padwidthimg = maxdim - imgshape
#         padwidthnew = maxdim - newsize

#         imgpad = np.pad( img, padwidthimg, mode='constant', constant_values=0 )
#         imgonespad = np.pad( imgones, padwidthimg, mode='constant', constant_values=2 )
#         imgnewonespad = np.pad( newones, padwidthnew, mode='constant', constant_values=0 )
        
#         imgnewidx = imgonespad * imgnewonespad

#         imgonespad[imgonespad]
#         newimg = np.zeros( maxdim, dtype=img.dtype )

#         if padval == None:

#             padval = np.mean( img )

#         newimg[imgnewidx == 2] = padval
#         newimg[imgnewidx == 1] = imgpad[imgnewidx == 1]