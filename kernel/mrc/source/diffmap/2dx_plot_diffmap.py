import sys
import os
from plotting import *



if __name__ == '__main__':
	no_args = len(sys.argv)
	if no_args < 4:
		sys.exit('Usage: python '+sys.argv[0]+' <map1> <map1-sig> <map2-sig>')
	map1_filepath=sys.argv[1]
	diffmap1_filepath=sys.argv[2]
	diffmap2_filepath=sys.argv[3]
	header = []
	with open(map1_filepath,'r') as mrcFile:
		 im1 = MRCImage(mrcFile)	
	with open(diffmap1_filepath,'r') as mrcFile:
		 im1sig = MRCImage(mrcFile)	
	with open(diffmap2_filepath,'r') as mrcFile:
		 im2sig = MRCImage(mrcFile)
        [width, height] = cropImages(im1sig,im2sig)
        cropImage(im1, width, height)
        images = [im1sig, im2sig]
	max_val = scaleImages(images)
        print "maxval is "+str(max_val)
	cutImages(images)
	plotImage(images[0].image, max_val, "map1")
	saveImage(images[0])
	plotImage(images[1].image, max_val, "map2")
	saveImage(images[1])
        contour = im1.image
        diffmap = getDiffmap(images[0],images[1])
	if no_args < 6:
	    plotDiffmap(contour,diffmap)
	elif no_args == 6:
	    plotDiffmap(contour,diffmap, sys.argv[4], sys.argv[5])
        else:
	    plotDiffmap(contour,diffmap, sys.argv[4], sys.argv[5], sys.argv[6])
	plt.show()

	

	

	