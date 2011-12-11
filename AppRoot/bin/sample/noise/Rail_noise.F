C*******************************************************
C    To Calculate Sound Pressure at Exterior Points      
C*******************************************************
C
C  By Wenwei Chen
C
C 

      PROGRAM  RAILNOISE
      IMPLICIT NONE

C VARIABLE DECLARATION
C --------------------

C  PARAMETERs for storing the limits on the dimension of arrays
C   Limit on the number of elements
      INTEGER    MAXNELM
      PARAMETER (MAXNELM=12)
C   Limit on the number of vertices (equal to the number of elements)
      INTEGER    MAXGNUM
      PARAMETER (MAXGNUM=14)
C   Limit on the number of acoustic frequencies
      INTEGER    MAXNFRQ
      PARAMETER (MAXNFRQ=1000)
C     
      INTEGER    MAXNTS
      PARAMETER (MAXNTS=100)      
C   Limit on the number of points exterior to the plate, where 
C    acoustic properties are sought
      INTEGER    MAXPNTS
      PARAMETER (MAXPNTS=10)
C  Constants
C   Real scalars: 0, 1, 2, pi
      REAL*8 ZERO,ONE,TWO,THREE,FOUR,PI
C   Complex scalars: (0,0), (1,0), (0,1)
      COMPLEX*16 CZERO,CONE,CIMAG

C  The reference pressure, used to convert units to decibels.
      REAL*8     REFPRS
C
C  Properties of the acoustic medium
C 
      REAL*8      CVAL(MAXNFRQ),RHOVAL(MAXNFRQ),Waven
C   Angular frequency 
      COMPLEX*16 OMEGA

C  Geometrical description of the plate(ies)
C   Number of elements and counter
      INTEGER    NELM,IPI
C   Number of collocation points (on \Pi) and counter
      INTEGER    NPIP,ISP
C   Number of vetices and counter
      INTEGER    GNUM,IV
C   Index of nodal coordinate for defining boundaries (standard unit is 
C    metres)
      REAL*8     GRID(MAXGNUM,3)
C   The three nodes that define each element on the boundaries
      INTEGER    TRIA3(MAXNELM,3)
C
C    Number of exterior points and counter
C
      INTEGER    NEPNTS,IPE
C    Coordinates of the exterior points
      REAL*8     CORD_PNTS(MAXPNTS,3)

C    Number of test problems and counter
      INTEGER    NFRQ,IFRC

C    FRVAL(i) is assigned the
C    acoustic frequency of the i-th test problem.
      REAL*8     FRVAL(MAXNFRQ), KVAL(MAXNFRQ), TS
C
      COMPLEX*16 PIALVAL(MAXNFRQ,MAXNELM),PIBEVAL(MAXNFRQ,MAXNELM)
C           
C   The actual plate condition is specified by assigning values to 
C    the data structure PIFVAL. 
C    PIFVAL(i,j) is assigned the value of f at the center of the j-th 
C    element for the i-th test problem.
C
      COMPLEX*16 PIFVAL(MAXNFRQ,MAXNELM)

C
C    ALPHA(j) is assigned the value of {\alpha} at the centre of the 
C     j-th element.
C
      COMPLEX*16 ALPHA(MAXNELM),BETA(MAXNELM),PIFUN(MAXNELM)
C
C  Validation and control parameters for RALEIGH
C   Switch for particular solution
      LOGICAL    LSOL,LVALID
C  
C   The maximum absolute error in the parameters that describe the
C    geometry of the plate.
      REAL*8     EGEOM
C
      REAL*8     REAL_Part(MAXNFRQ,MAXGNUM), IMAG_Part(MAXNFRQ,MAXGNUM)
      REAL*8     RLP(MAXNFRQ,MAXNELM), IMP(MAXNFRQ,MAXNELM)
C Output from subroutine RALEIGH
      COMPLEX*16 PHI_PLT(MAXNELM),VEL_PLT(MAXNELM),PHI_EPNTS(MAXPNTS)
C
C Workspace for RALEIGH
C
      COMPLEX*16 WORK(MAXNELM,MAXNELM)
      COMPLEX*16 WORK2(MAXPNTS,MAXNELM)
      COMPLEX*16 WORK3(MAXNELM,MAXNELM)
      COMPLEX*16 WORK4(MAXNELM)
      COMPLEX*16 WORK5(MAXNELM)
      LOGICAL    WORKFG(MAXNELM)

C  Local storage of pressure, pressure/velocity 
      COMPLEX*16 PRESSUREC(MAXPNTS,MAXNFRQ),TEMPC
      REAL*8     PRESSURE(MAXPNTS,MAXNFRQ),TEMP
      REAL       PRES_RL(MAXNFRQ),PRES_IM(MAXNFRQ),PRES
C
      REAL*8     EPS, WIDTH, LENGTH , ELEMWD, ELEMLH
C
      INTEGER    IOUT
      CHARACTER*30 COUNTER,OUTPUT
C
C INITIALIZATION
C --------------

C Set constants
      ZERO=0.0D0
      ONE=1.0D0
      TWO=2.0D0
      THREE=3.0D0
      FOUR=4.0D0
      PI=4.0D0*ATAN(ONE)
      CZERO=CMPLX(ZERO,ZERO)
      CONE=CMPLX(ONE,ZERO)
      CIMAG=CMPLX(ZERO,ONE)
      EPS=1.0E-10
	WIDTH = 3.0D0
	LENGTH = 36.0D0
	ELEMWD = 3.0D0
	ELEMLH = 6.0D0
C
C  Reference for decibel scales
C
      REFPRS=2.0D-05
C Describe the nodes and elements that make up the plate
C  : Centred at the point (0.0,0.0,0.0) and lying in the
C  : x-y plane is divided into NELM uniform elements. GRID and TRIA3 
C  : are defined anti-clockwise around the plate so that the normal to 
C  : the plate is assumed to be outward
C  : Set coordinates of the nodes
C  : Set up GRID, the coordinates of the vertices of the elements
C
      GNUM=14
	GNUM=GNUM/2
C
      DO IV = 1, GNUM
	   GRID(IV,1) = -LENGTH/TWO + (IV-1)*ELEMLH
	   GRID(IV,2) =  WIDTH/TWO
	   GRID(IV,3) =  ZERO
C
         GRID(GNUM+IV,1) = -LENGTH/TWO + (IV-1)*ELEMLH
	   GRID(GNUM+IV,2) = -WIDTH/TWO
	   GRID(GNUM+IV,3) =  ZERO
      ENDDO
C
      NELM = (GNUM - 1)*2
C  : Set nodal indices that describe the elements of the plate
C  :  The indices refer to the nodes in GRID. The order of the
C  :  nodes in TRIA3 dictates that the normal is outward from the 
C  :  plate into the acoustic domain.
C
C     ELEMENTS & NODES
C
C     1*\  2*\   3*\   4*\   5*\   6*\   7*
C        \    \     \     \     \     \
C     8*  \9*  \10*  \11*  \12*  \13*__\14*
      NELM = NELM/2
	DO IPI = 1, NELM
C
	   TRIA3(2*IPI-1,1) =(IPI-1) + 1
	   TRIA3(2*IPI-1,2) =(IPI-1) + GNUM+1
	   TRIA3(2*IPI-1,3) =(IPI-1) + GNUM+2
C
         TRIA3(2*IPI,1) =(IPI-1) + 2
	   TRIA3(2*IPI,2) =(IPI-1) + 1
	   TRIA3(2*IPI,3) =(IPI-1) + GNUM+2
	ENDDO    	    


C Set NEPNTS=100 and set the exterior points to be the points on the axis
C  of the vibrating piston
      NEPNTS=10
      DO 110 IPE=1,NEPNTS
        CORD_PNTS(IPE,1)=ZERO
        CORD_PNTS(IPE,2)=ZERO
        CORD_PNTS(IPE,3)=2.0D0+DFLOAT(IPE)
110   CONTINUE

C The number of points on the plate is equal to the number of 
C  elements
      NPIP=MAXNELM
        
C Set up test problems
C  :Set the number of test problems
      NFRQ=MAXNFRQ
C READ POINTS FREQUENCY
      CALL ReadVelo (FRVAL,Real_Part,Imag_Part,MAXNFRQ,MAXGNUM)
C GET CENTRAL VALUE FOR EVERY ELEMENT
      DO IFRC=1, NFRQ
         DO IPI = 1, NPIP
            RLP(IFRC,IPI)= (Real_Part(IFRC,TRIA3(IPI,1)) +
     1                       Real_Part(IFRC,TRIA3(IPI,2)) +
     2                       Real_Part(IFRC,TRIA3(IPI,3)))/THREE
C
            IMP(IFRC,IPI)= (Imag_Part(IFRC,TRIA3(IPI,1)) +
     1                       Imag_Part(IFRC,TRIA3(IPI,2)) +
     2                       Imag_Part(IFRC,TRIA3(IPI,3)))/THREE
         ENDDO
      ENDDO   
C      
      DO 200 IFRC=1,NFRQ

C  Properties of the acoustic medium. C the propagation velocity
C  and RHO the density of the acoustic medium. C>0, RHO>0
C  :Acoustic medium is air at 20 celcius and 1 atmosphere. 
C  [C in metres per second, RHO in kilograms per cubic metre.]
      CVAL(IFRC)=344.0D0
      RHOVAL(IFRC)=1.205D0

C  :Set acoustic frequency value (hertz) in FRVAL
      KVAL(IFRC)=2.0D0*PI*FRVAL(IFRC)/CVAL(IFRC)

C  :Set nature of the plate condition by prescribing the values of
C   the plate functions PIALVAL and PIBEVAL at the collocation points
C   :In this case a Dirichlet (phi-valued) plate condition
      DO 160 ISP=1,NPIP
        PIALVAL(IFRC,ISP)=CZERO
        PIBEVAL(IFRC,ISP)=CONE
        PIFVAL(IFRC,ISP)=CMPLX(RLP(IFRC,ISP),IMP(IFRC,ISP))
C        PIFVAL(IFRC,ISP)=SQRT(RLP(IFRC,ISP)**2+IMP(IFRC,ISP)**2)
160   CONTINUE

200   CONTINUE

C  :Switch for particular solution
      LSOL=.TRUE.
C  :Switch on the validation of RALEIGH
      LVALID=.TRUE.
C  :Set EGEOM
      EGEOM=1.0D-4
C
C Loop(IFRC) through the test problems
C
      DO 300 IFRC=1,NFRQ
C  Set OMEGA, the angular frequency omega and Waven, the wavenumber
        Waven=KVAL(IFRC)
        OMEGA=2.0D0*PI*FRVAL(IFRC)

C   Set up particular alpha and beta functions for this wavenumber
C    and type of plate condition
        DO ISP=1,NPIP
          ALPHA(ISP)=PIALVAL(IFRC,ISP)
          BETA(ISP)=PIBEVAL(IFRC,ISP)
          PIFUN(ISP)=PIFVAL(IFRC,ISP)
        ENDDO
       
        CALL RALEIGH(Waven,MAXGNUM,GNUM,GRID,MAXNELM,NELM,TRIA3,
     1               MAXPNTS,NEPNTS,CORD_PNTS,ALPHA,BETA,PIFUN,
     2               LSOL,LVALID,EGEOM,PHI_PLT,VEL_PLT,PHI_EPNTS,
     3               WORK,WORK2,WORK3,WORK4,WORK5,WORKFG)
C Compute
        DO IPE=1,NEPNTS
            TEMPC = CIMAG*RHOVAL(IFRC)*OMEGA*PHI_EPNTS(IPE)
            PRESSUREC(IPE,IFRC)=TEMPC
            TEMP = SQRT(REAL(TEMPC)**2+IMAG(TEMPC)**2)
            TEMP = 20*log10(TEMP/REFPRS)
            PRESSURE(IPE,IFRC) = TEMP
        ENDDO
C
C  Close loop(IFRC) through the test problems
300   CONTINUE
C
C  Output the solutions
C
C  Open file for the output data
      IOUT = 10
C
      DO IPE = 1, NEPNTS
C
         WRITE( COUNTER,'(I5)' ) IPE
C         
         IF (IPE .LT. 10) THEN
            OUTPUT = 'Rail_Noise_Freq0'//trim(adjustl(COUNTER))//'.out'
         ELSE
            OUTPUT = 'Rail_Noise_Freq'//trim(adjustl(COUNTER))//'.out'
         ENDIF
C            
         OPEN(IOUT,FILE=OUTPUT)      
C  Formats for output
         WRITE(IOUT,*) 'HEIGHT = ',CORD_PNTS(IPE,3)
C        
         WRITE(IOUT,*) 'Frequency   Sound Pressure(decibal)' 
         DO  IFRC=2,NFRQ
            WRITE(IOUT,900) FRVAL(IFRC),PRESSURE(IPE,IFRC)
         ENDDO
      ENDDO
C        
      CLOSE(IOUT)

900   FORMAT(2F14.8)
C
C     PERFORM INVERSE FFT FOR SOUND PRESSUE IN THE FREQUENCY FIELD
C
      DO IPE = 1, NEPNTS
C
         DO  IFRC=1,NFRQ
             PRES_RL(IFRC)=REAL(PRESSUREC(IPE,IFRC))
             PRES_IM(IFRC)=IMAG(PRESSUREC(IPE,IFRC))
         ENDDO
C
         TS = (1.0/(FRVAL(3)-FRVAL(2)))/NFRQ                   
         CALL FFT(PRES_RL,PRES_IM,NFRQ,NFRQ,NFRQ,-1)
C
         IOUT = 20
C
         WRITE( COUNTER,'(I5)' ) IPE
C
         IF (IPE .LT. 10) THEN
            OUTPUT = 'Rail_Noise_Time0'//trim(adjustl(COUNTER))//'.out'
         ELSE
            OUTPUT = 'Rail_Noise_Time'//trim(adjustl(COUNTER))//'.out'
         ENDIF
C         
         OPEN(IOUT,FILE=OUTPUT)      
C  Formats for output
         WRITE(IOUT,*) 'HEIGHT = ',CORD_PNTS(IPE,3)
C        
         WRITE(IOUT,*) 'Time        Sound Pressure(decibal)' 
         DO  IFRC=1,NFRQ
            PRES = SQRT(PRES_RL(IFRC)**2+PRES_IM(IFRC)**2)
            PRES = 20*log10(PRES/REFPRS)
            WRITE(IOUT,900) IFRC*TS,PRES
         ENDDO         
      ENDDO
C
      END 
