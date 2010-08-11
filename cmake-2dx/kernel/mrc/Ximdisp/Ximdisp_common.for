C***	Ximdisp_common.for
C***
	real*4		bignum
	parameter	(bignum = 10.E+10)
	real*4		degrad
	parameter	(degrad = 0.017453292)
	integer*4	iautoinc
	parameter	(iautoinc = 5)
C*** set same as border size in Ximagelibc.c
	integer*4	iborder
	parameter	(iborder = 50)
	integer*4	ibox_size
	parameter	(ibox_size = 3)
	integer*4	icross_size
	parameter	(icross_size = 6)
	integer*4	idevavg
        parameter       (idevavg = 10)
	integer*4	idevcol
        parameter       (idevcol = 9)
	integer*4	idevcoord
        parameter       (idevcoord = 8)
	integer*4	idevdef
	parameter	(idevdef = 11)
	integer*4	idevlat
        parameter       (idevlat = 7)
	integer*4	idevmap
        parameter       (idevmap = 1)
	integer*4	idevout
        parameter       (idevout = 4)
	integer*4	idevpost
	parameter	(idevpost = 3)
	integer*4	iprint
        parameter       (iprint = 0)
	integer*4	izoom_default
        parameter       (izoom_default = 4)
	integer*4	izoom_length_default
        parameter       (izoom_length_default = 256)
C*** changed from 125 to 123 to allow top bits for vector drawing 03.08.05
	real*4		grey
	parameter	(grey = 127.0 - 4.0)
	real*4		greymin
	parameter	(greymin = 2.0)
	real*4		greymax
	parameter	(greymax = 20.0)
	real*4		greyrange
	parameter	(greyrange = greymax - greymin)
	integer*4	max_coords
	parameter	(max_coords = 16384)
	integer*4	max_colour
        parameter       (max_colour = 65535)
	integer*4	min_den
        parameter       (min_den = 0)
	integer*4	max_den
        parameter       (max_den = 127)
	integer*4	max_fonts
	parameter	(max_fonts = 100)
	integer*4	max_items
        parameter       (max_items = 20)
	integer*4	max_lat
	parameter	(max_lat = 100)
	integer*4	max_lines_per_page
	parameter	(max_lines_per_page=30)
	integer*4	max_phasebuf
	parameter	(max_phasebuf = 3000 * 2050)
	integer*4	max_points
	parameter	(max_points = 1000)
	integer*4	max_range
	parameter	(max_range = max_den - min_den + 1)
	integer*4	max_overlay_width
	parameter	(max_overlay_width = 1026)
	integer*4	max_overlay
	parameter	(max_overlay = max_overlay_width * max_overlay_width)
	integer*4	max_spline_width
	parameter	(max_spline_width = 2048)
	integer*4	max_spline_height
	parameter	(max_spline_height = 4096)
	integer*4	max_width
	parameter	(max_width = 50000)
	integer*4	minbox
	parameter	(minbox = 3)
	integer*4	nphasecolours
	parameter	(nphasecolours = 8)
	integer*4	nsdevs
	parameter	(nsdevs = 6)
        integer*4       nsecsperday
        parameter       (nsecsperday = 24 * 60 * 60)
        integer*4       ntrialdays
        parameter       (ntrialdays = 370)
	real*4		one
	parameter	(one = 1.0)
	real*4		pi
	parameter	(pi = 3.141593)
	real*4		radcon
        parameter	(radcon = 180. / pi)
	real*4		radians
	parameter	(radians = pi / 180.)
	integer*4	screen_width
        parameter       (screen_width = 8192)
	integer*4	screen_height
        parameter       (screen_height = 8192)
	integer*4	screen_size
        parameter       (screen_size = screen_width * screen_height)
	integer*4	text_shift
	parameter	(text_shift = 20)
	real*4		varmin
	parameter	(varmin = 0.00001)

C*** function declarations
	integer*4	chtoin
	integer*4	lnblank
C***
	real*8		mapreal8(0:screen_size / 8 - 1)
	real*8		wd(max_points)
	real*8		rd(max_points)
	real*8		xn(max_points)
	real*8		fn(max_points)
	real*8		gn(max_points)
	real*8		dn(max_points)
	real*8		theta(max_points)
	real*8		w(max_spline_height)
	real*8		xden(max_points)
	real*8		yden(max_points)
	real*8		tg01bd
	real*8		xspline
	real*8		yspline
C***
	integer*4       ih(max_points)
	integer*4       ik(max_points)
	integer*4	idevin
        integer*4          ixlat
     *                  (-max_lat:max_lat,-max_lat:max_lat)
        integer*4          iylat
     *                  (-max_lat:max_lat,-max_lat:max_lat)
        integer*4       iphasecolours(3,0:7)
        integer*4       ixcomp(max_points)
        integer*4       iycomp(max_points)
        integer*4       ixp(max_points)
        integer*4       iyp(max_points)
        integer*4       ixv1(max_points)
        integer*4       iyv1(max_points)
        integer*4       ixv2(max_points)
        integer*4       iyv2(max_points)
	integer*4	ixspline(max_spline_height)
	integer*4	iyspline(max_spline_height)
	integer*4	ixcoord(max_coords)
	integer*4	iycoord(max_coords)
	integer*4       ixyzmin(3)
	integer*4       ixyzmax(3)
	integer*4       ixyz(3)
	integer*4       kxyz(3)
	integer*4       mxyz(3)
	integer*4       nxyz(3)
        integer*4       red(0:255)
        integer*4       blue(0:255)
        integer*4       green(0:255)
C***
        logical         amp
        logical         avout
        logical   	auto
	logical		autolabel
	logical		autoscale
	logical 	big
	logical 	blackwhite
        logical 	box_inside
	logical		circ
	logical		flip
	logical		poly
	logical		rect
	logical		coordsout
	logical		colour
        logical         dump_screen
        logical         first
        logical         floatim
	logical         ifirst
        logical   	image
        logical   	imagic
	logical		invert
	logical		interactive
        logical         mapread
        logical         montage
        logical         multisection
        logical   	newmap
	logical		old
	logical		outputcoords
	logical		outswitch
	logical		phasemap
	logical		pointer
	logical		postscript
	logical         primary
	logical		redgrn
        logical   	redisplay
	logical		refined
	logical		rescale
        logical         standard
        logical         screen
        logical         spider
	logical		stack
        logical   	there
        logical   	trial
        logical         vbox
C***
	real*4		aline(max_width)
        real*4          cell(6)
	real*4		chunk(max_spline_width,max_spline_height)
	real*4		density(max_width)
	real*4		colour_compon
	real*4		colour_icurve
        real*4          xd(max_points)
        real*4          yd(max_points)
	real*4		delx(max_spline_height)
	real*4		alpha(max_spline_height)
	real*4		mapout
     *                  (max_spline_width * max_spline_height)
	real*4		phaseangle(max_width)
        real*4          dist_lat
     *                  (-max_lat:max_lat,-max_lat:max_lat)
        real*4          xn_lat
     *                  (-max_lat:max_lat,-max_lat:max_lat)
        real*4          yn_lat
     *                  (-max_lat:max_lat,-max_lat:max_lat)
	real*4		transform(max_overlay)
C***
        integer*2       ibox(screen_height,2)
	integer*2	iden(max_spline_width,max_spline_height)
C***
	byte		buf(2)
        byte      	mapbuf(0:screen_size-1)
	byte		phasebuf(0:max_phasebuf-1)
	byte		transbuf(0:max_overlay-1)
C***
	character	iolabel(max_lines_per_page)*80
	character	return_string*80
C***
        character       avgfile*256
        character       boxfile*256
	character	char
        character       choice*4
	character	colour_file*256
	character	coordfile*256
	character	defaultfile*256
	character	date*24
	character	fontlist(max_fonts)*64
	character	intoch*256
        character       keyword*80
	character	infile*256
	character	latfile*256
        character       libspec*80
	character       mapfile*256
	character	menulist(max_items)*64
	character	outputfile*256
	character	rmblank*256
	character	string*256
	character	switch*2
	character	time*8
	character	title*80
        character       yesno
        character       ylc
C***
	equivalence	(mapbuf(0),mapreal8(0))
	equivalence	(mapout,chunk)
C***
	common/map/
     *         mapbuf,phasebuf
C***
	common/gen/
     *         auto,big,image,newmap,old,redisplay,rescale,outswitch,
     *         pointer,ifirst,multisection,montage,
     *	       idevin,icenx,iceny,icx,icy,ierr,ixm,iym,
     *         ixcr,iycr,ixmin,ixmax,iymin,iymax,iyheight,
     *	       max_display_width, max_display_height,
     *         max_screen_width, max_screen_height,
     *	       mode,imap,nmaps,icompress,
     *         nlabels,nox,noy,noz,numsecx,numsecy,
     *         nxstart,nystart,nzstart,nxend,nyend,nzend,
     *         nxyz,
     *         amin,amax,dmean,scl,startmin,startmax,
     *	       iolabel
C***
	common/avg/avout,nboxes,npixels,numxy,
     *             amean,bmean,ameantot,bmeantot
C***
        common/box/floatim,screen,dump_screen,invert,postscript,
     *		   circ,poly,rect,
     *             ixp,iyp,npts,mcircx,mcircy,radius,
     *             nxpad,nypad,ncenx,nceny,
     *             mnx,mxx,mny,mxy,numx,numy,mwidth,mheight,
     *             nblocks,lblock,iysub,aline,ibox,
     *             boxfile
C***
	common/colour/colourscale, fmaxcolour, outputcoords,
     *                bmin, bmax
C***
	common/coords/ixspot,iyspot,iwid,nspot,isymb,autolabel,
     *			alength,angle
C***
        common/fft/rotang,nxshift,nyshift,left,iphasex,iphasey,
     *                amplitude,phase,flip,pshift,delpx,delpy,
     *		      mapcenx, mapceny
C***
        common/latref/nspots,irang,
     *                ixlat,iylat,
     *                x0,y0,x1,y1,x2,y2,aln,bln,aang,bang,
     *                cosinc,refined,
     *                ih,ik,xd,yd,dist_lat,xn_lat,yn_lat,
     *                mapfile,latfile
C***
        common/lktble/ixst,iyst,first,phasemap
C***
	common/ovly/amp,autoscale,
     *                nxbox,nybox,nxtrans,nytrans,ixpos,iypos,
     *		      boxmin,boxmax,boxmean,
     *		      transmin,transmax,transmean,transdev,
     *		      trmin,trmax,
     *                transform,transbuf
C***
	common/spline/mapout,iden,
     *                itransx,itransy,
     *                xden,yden,
     *                wd,rd,xn,fn,gn,dn,theta
C***
	data outputfile/'Ximdisp.tmp'/
C*** colours red,orange,yellow,green,cyan,blue,violet,magenta
        data            iphasecolours/65535,16384,16384,
     *                                65535,32767,8192,
     *                                65535,65535,8192,
     *                                    0,65535,32767,
     *                                32767,65535,65535,
     *                                32767,49151,65535,
     *                                49151,32767,65535,
     *                                65535,0,65535/
C*** edit next line for trial version.
C*** run /ss0/jms/X/alf/getdate to get number of seconds, modify trial to true
        data nstartdate/926508244/,trial/.false./
c        data nstartdate/926508244/,trial/.true./

