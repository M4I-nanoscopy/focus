C	PROGRAM TO COMPUTE CTF CURVES FOR CHOSEN VALUES OF
C	CS; KV; DEFOCUS RANGE;
C
C		Original program	JMB	15.1.1985
C		to Dec Alpha		 RH	 1.2.1996
C	V1.1 	update to use plot2000	 RH	14.8.2000
C       "       P2K_FONT fix             TSH    13.6.2001
C
C	Control cards
C
C	1.	KV,CS			voltage, spherical aberration
C	2.	DSSTEP,DSMAX		reciprocal Angstrom step and maximum
C	3.	DFSTART,DFEND,DFSTEP	defocus start, finish and interval
C					Note that the number of plot pages will
C					be (DFEND-DFSTART)/DFSTEP.
C
	DIMENSION X(1000),Y(1000)
	REAL KV
C
      CALL P2K_OUTFILE('CTFCALC.PS',10)
C	
        READ(5,*)KV,CS
	WRITE(6,10)KV,CS
10	FORMAT(' ACCELERATING VOLTAGE (KV) =',F8.1/
     .	' SPHERICAL ABERRATION (MM) =',F8.2)
C	COMPUTE WAVELENGTH
	CSA=CS*(10.0**7.0)
	V=KV*1000.
	WL=12.3/SQRT(V+V**2/(10.0**6.0))
	WRITE(6,11)WL
11	FORMAT(' WAVELENGTH (ANGSTROMS) =',F8.4)
	READ (5,*)DSSTEP,DSMAX
	WRITE(6,13)DSSTEP,DSMAX
13	FORMAT(' STEP SIZE (1/ANGSTROMS) FOR COMPUTATION OF CTF =',  
     .	F10.4/' CTF COMPUTED OUT TO ',F10.3)
	READ(5,*)DFSTART,DFEND,DFSTEP
	WRITE(6,12)DFSTART,DFEND,DFSTEP
12	FORMAT(' COMPUTE CTF FOR DEFOCUS VALUES (ANGSTROMS) FROM',
     .	F10.1,' TO',F10.1,' IN STEPS OF',F10.3//)
C
	PI=3.1415926
	C1=(2*PI)/WL
	C3=(WL**2)/2
C
	IDF=(DFEND-DFSTART)/DFSTEP + 1.0

	IDSMAX=DSMAX/DSSTEP + 1.0
	C2=CSA*(C3**2)
C
	DO 50 I=1,IDF
	DF=DFSTART+ (I-1)*DFSTEP
	WRITE(6,51)DF
51	FORMAT(' VALUE OF DEFOCUS =',F10.1)
C	
	C4=C3*DF
	DO 100 J=1,IDSMAX
	DS=(J-1)*DSSTEP
	X(J)=DS
	DS2=DS*DS
	DS4=DS2*DS2
	G=C1*(C2*DS4-C4*DS2)
	Y(J)=2*SIN(G)
100	 CONTINUE
C
	CALL PLOTCTF(DF,CS,KV,WL,DSMAX,DSSTEP,IDSMAX,X,Y)
C
50	CONTINUE
C
200   WRITE(6,201)
201   FORMAT(' PLOTS FINISHED')
	STOP
	END
C
C
	SUBROUTINE PLOTCTF(DF,CS,KV,WL,DSMAX,DSSTEP,IDSMAX,X,Y)
CTSH	DIMENSION X(1000),Y(1000),TEXT(20)
CTSH++
	DIMENSION X(1000),Y(1000)
        CHARACTER*80 TEXT
CTSH--
	REAL KV
C
C
      DIMENSION TITLE(10)
C
C
C     
      FONTSIZE=4.0      ! SELECT 4MM CHAR HEIGHT FOR TEXT
      PLTSIZ=226.5	! this was originally the paper size in mm
      ANGCHAR=90.0 	! may be changed if plot2000 changes
      CALL P2K_HOME
      CALL P2K_FONT('Courier'//CHAR(0),FONTSIZE)
      CALL P2K_COLOUR(0)
      CALL P2K_GRID(0.5*PLTSIZ,0.5*PLTSIZ,1.0)
      CALL P2K_TWIST(90.0,180.0,120.0) 	! rotates xy-axes to landscape mode
      CALL P2K_ORIGIN(-0.643*PLTSIZ,-0.063*PLTSIZ,0.) ! origin left below centre
      CALL P2K_MOVE(0.,-76.5,0.)
CTSH      WRITE(TEXT,512)CS,KV,WL
CTSH++
      WRITE(TEXT,512)CS,KV,WL
CTSH--
512   FORMAT(' CS;KV;WL',F5.2,F5.0,F7.4)
      CALL P2K_STRING(TEXT,26,ANGCHAR)
      CALL P2K_MOVE(0.,-66.5,0.)
CTSH      WRITE(TEXT,508)DF
CTSH++
      WRITE(TEXT,508)DF
CTSH--
508   FORMAT(' DEFOCUS',F8.0)
      CALL P2K_STRING(TEXT,16,ANGCHAR)
      CALL P2K_MOVE(0.,0.,0.)
      CALL P2K_DRAW(300.,0.,0.)
      CALL P2K_MOVE(0.,-81.,0.)
      CALL P2K_DRAW(0.,81.,0.)
C
      CALL P2K_DRAW(300.,81.,0.)
      CALL P2K_DRAW(300.,-81.,0.)
      CALL P2K_DRAW(0.,-81.,0.)
C
      XSCALE=300./DSMAX
      YSCALE=81./3.0
C
      DO 1130 I=1,10
      DO 1120 J=1,2
      C=I
      D=J
      C=(-1.)+C
      D=(-1.)+D
      E=0.- 81.
      F=-(1.5*(1.+D))-81.
      E0=0.
      F0=-(1.5*(1.+D))
      E1=0.+81.
      F1=-(1.5*(1.+D))+81.
      G=15.+(30.*C)+(15.*D)
      CALL P2K_MOVE(G,E,0.)
      CALL P2K_DRAW(G,F,0.)
      CALL P2K_MOVE(G,E0,0.)
      CALL P2K_DRAW(G,F0,0.)
      CALL P2K_MOVE(G,E1,0.)
      CALL P2K_DRAW(G,F1,0.)
1120  CONTINUE
1130  CONTINUE
C
C      YPOS=-87.
      YPOS=-90.
      XPOS=-30.
	ZSTEP=DSMAX/10.
      ZST=-ZSTEP
      DO 507 IZ=1,11
      XPOS=XPOS+30.
      ZST=ZST+ZSTEP
      CALL P2K_MOVE(XPOS,YPOS,0.)
CTSH      WRITE(TEXT,509)ZST
CTSH++
      WRITE(TEXT,509)ZST
CTSH--
509   FORMAT(F4.2)
      CALL P2K_CSTRING(TEXT,4,ANGCHAR)
507   CONTINUE
C
C	YPOS=-95.
	YPOS=-96.5
      XPOS=250.
      CALL P2K_MOVE(XPOS,YPOS,0.)
CTSH      WRITE(TEXT,510)
CTSH++
      WRITE(TEXT,510)
CTSH--
510   FORMAT('(1/A)')
      CALL P2K_CSTRING(TEXT,5,ANGCHAR)
C
      STEP=27.
      XPOS=0.
      XPOS1=1.5
      XPOS2=-20.
      YPOS=-81.
      DO 500 IY=1,5
      YV=-2.0 + (IY-1)
      YPOS=YPOS+STEP
      CALL P2K_MOVE(XPOS,YPOS,0.)
      CALL P2K_DRAW(XPOS1,YPOS,0.)
      CALL P2K_MOVE(XPOS2,YPOS-1.5,0.)
CTSH      WRITE(TEXT,501)YV
CTSH++
      WRITE(TEXT,501)YV
CTSH--
501   FORMAT(F5.1)
      CALL P2K_STRING(TEXT,5,ANGCHAR)
500   CONTINUE
C
C
C
	ICP=0
C     PLOT CURVE
      XP=X(1)*XSCALE
      YP=Y(1)*YSCALE
      IF(YP.GT.81.)YP=81.
C      
      IF(YP.LT.-81.)YP=-81.
      ICP=ICP+1
      CALL P2K_MOVE(XP,YP,0.)
C
      DO 410 I=2,IDSMAX
      ICP=ICP+1
      XP=X(I)*XSCALE
      YP=YSCALE*Y(I)
      IF(YP.GT.81.)YP=81.
      IF(YP.LT.-81.)YP=-81.
      CALL P2K_DRAW(XP,YP,0.)
410   CONTINUE
411   CONTINUE
C
401   CALL P2K_PAGE
C
C
C
C
	WRITE(6,1)DF,CS,KV,WL,DSMAX,DSSTEP
1	FORMAT(6F12.4)
	RETURN
	END
