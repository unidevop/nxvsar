C***************************************************************
C
C This subroutine computes the solution to the three-dimensional 
C Helmholtz equation
C                  __ 2                2
C                  \/    {\phi}   +   k  {\phi}   =  0   
C
C in the domain exterior to a plate in an infinite rigid baffle.
C
C The subroutine

      SUBROUTINE RALEIGH(Wavenum,MAXGNUM,GNUM,GRID,MAXNELM,NELM,TRIA3,
     1                    MAXPNTS,COMPNTS,CORD_PNTS,ALPHA,BETA,FUNFV,
     2                    SOLFG,VALIDFG,GEOMERR,PHI_PLATE,VEL_PLATE,
     3                    PHI_EXT,WORK1,WORK2,WORK3,WORK4,WORK5,WORKFLG)
C     
      PARAMETER (MAXNQ=100)

C  Wavenumber
      REAL*8     Wavenum

C  Boundary geometry
C  Number of grids
      INTEGER    MAXGNUM,GNUM
C   The coordinates of the vertices on {\Pi}
      REAL*8     GRID(MAXGNUM,3)
C   Limit on the number of elements describing {\Pi}
      INTEGER    MAXNELM,NELM
C   The indices of the vertices describing each element
      INTEGER    TRIA3(MAXNELM,3)    
C  Exterior points at which the solution is to be observed
C   Limit on the number of points exterior to the plate where 
C    solution is sought
      INTEGER    MAXPNTS,COMPNTS   
C   Coordinates of the exterior points
      REAL*8     CORD_PNTS(MAXPNTS,3)
C  The plate condition is such that {\alpha} {\phi} + {\beta} v = f
C  where alpha, beta and f are complex valued functions over {\Pi}.
C  The functions are set values at the collocation points.
C   function alpha
      COMPLEX*16 ALPHA(MAXNELM),BETA(MAXNELM),FUNFV(MAXNELM)
C  Validation and control parameters
      LOGICAL    SOLFG,VALIDFG   
      REAL*8     GEOMERR
C  Solution 
C   function phi
      COMPLEX*16 PHI_PLATE(MAXNELM),PHI_EXT(MAXPNTS),VEL_PLATE(MAXNELM)
C  Working space 
      COMPLEX*16 WORK1(MAXNELM,MAXNELM),WORK2(MAXPNTS,MAXNELM)
      COMPLEX*16 WORK3(MAXNELM,MAXNELM),WORK4(MAXNELM),WORK5(MAXNELM)
      LOGICAL    WORKFLG(MAXNELM)

C  External function
      REAL*8     DIST3,AREA
C  Constants
C   Real scalars: 0, 1, 2, half, pi
      REAL*8 ZERO,ONE,TWO,THREE,HALF,THIRD,PI
C   Complex scalars: (0,0), (1,0), (0,1)
      COMPLEX*16 CZERO,CONE,CIMAG

C  Wavenumber in complex form
      COMPLEX*16 CK
C  Counter
      INTEGER    II,JJ,KK
C   Parameters for HELMDIF
      REAL*8     P(3),PA(3),PB(3),PC(3),QA(3),QB(3),QC(3),VECP(3)
      LOGICAL    LPONEL
C  Quadrature rule information
C   [Note that in this program two quadrature rules are used: one for
C    the case when the point P lies on the element (LPONEL=.TRUE.) and
C    one for the case when P does not lie on the element. In general,
C    it is more efficient to define a larger set of quadrature rules
C    so that a particular rule can be selected for any given point P 
C    and element QA-QB. For example using more quadrature points when
C    the element is large, less when the element is small, more when
C    the element is close to P, less when it is far from P.]
C   Quadrature rule used when LPONEL=.TRUE.
C    Number of quadrature points
      INTEGER    NQON
C    x-Abscissae of the actual quadrature rule
      REAL*8     XQON(MAXNQ),YQON(MAXNQ),WQON(MAXNQ)  
C   Quadrature rule used when LPONEL=.FALSE.
C    Number of quadrature points
      INTEGER    NQOFF
C    x-Abscissae of the actual quadrature rule
      REAL*8     XQOFF(MAXNQ),YQOFF(MAXNQ),WQOFF(MAXNQ)   
C   Quadrature rule parameters for HELMDIF
C    Actual number of quadrature points
      INTEGER    NQ
C    Abscissae of the actual quadrature rule
      REAL*8     XQ(MAXNQ),YQ(MAXNQ),WQ(MAXNQ)
C   Counter through the quadrature points
      INTEGER    IQ

C  Validation and control parameters for subroutine HELMDIF
      LOGICAL    LVAL
      REAL*8     EK,EQRULE    
      LOGICAL    LFAIL1,LLK,LMK,LMKT,LNK
C  Parameters for subroutine HELMDIF. 
      COMPLEX*16 DISLK,DISMK,DISMKT,DISNK
C  Other variables
C   Error flag
      LOGICAL    LERROR
C   Failure flag
      LOGICAL    LFAIL
C   Accumulation of solution {\phi}
      COMPLEX*16 SUMPHI
C   Maximum,minimum sizes of elements
      REAL*8     SIZMAX,SIZMIN,SIZE
C   The `diameter' of the plate or the maximum distance between any
C    three vertices
      REAL*8     DIAM
     
      REAL*8     NORMP(3),NORMQ(3)

C INITIALISATION
C --------------

C Set constants
      ZERO=0.0D0
      ONE=1.0D0
      TWO=2.0D0
      THREE=3.0D0
      HALF=ONE/TWO
      THIRD=ONE/THREE
      PI=4.0D0*ATAN(ONE)
      CZERO=CMPLX(ZERO,ZERO)
      CONE=CMPLX(ONE,ZERO)
      CIMAG=CMPLX(ZERO,ONE)

C Validation
C ==========

C Validation of parameters of RALEIGH
C ---------------------------------

      IF (VALIDFG) THEN

C Validation of main paramters
        LERROR=.FALSE.
        IF (Wavenum.LT.ZERO) THEN
          WRITE(*,*) 'Wavenum = ',Wavenum
          WRITE(*,*) 'ERROR(RALEIGH) - Wavenum must be positive'
          LERROR=.TRUE.
        END IF
        IF (MAXGNUM.LT.3) THEN
          WRITE(*,*) 'MAXGNUM = ',MAXGNUM
          WRITE(*,*) 'ERROR(RALEIGH) - must have MAXGNUM>=3'
          LERROR=.TRUE.
        END IF
        IF (GNUM.LT.3.OR.GNUM.GT.MAXGNUM) THEN
          WRITE(*,*) 'GNUM = ',GNUM
          WRITE(*,*) 'ERROR(RALEIGH) - must have 3<=GNUM<=MAXGNUM'
          LERROR=.TRUE.
        END IF
        IF (MAXNELM.LT.1) THEN
          WRITE(*,*) 'MAXNELM = ',MAXNELM
          WRITE(*,*) 'ERROR(RALEIGH) - must have MAXNELM>=1'
          LERROR=.TRUE.
        END IF
        IF (NELM.LT.1.OR.NELM.GT.MAXNELM) THEN
          WRITE(*,*) 'NELM = ',NELM
          WRITE(*,*) 'ERROR(RALEIGH) - must have 1<=NELM<=MAXNELM'
          LERROR=.TRUE.
        END IF
        IF (MAXPNTS.LT.1) THEN
          WRITE(*,*) 'MAXPNTS = ',MAXPNTS
          WRITE(*,*) 'ERROR(RALEIGH) - must have MAXPNTS>=1'
          LERROR=.TRUE.
        END IF
        IF (COMPNTS.LT.0.OR.COMPNTS.GT.MAXPNTS) THEN
          WRITE(*,*) 'COMPNTS = ',COMPNTS
          WRITE(*,*) 'ERROR(RALEIGH) - must have 0<=COMPNTS<=MAXPNTS'
          LERROR=.TRUE.
        END IF
        IF (GEOMERR.LE.ZERO) THEN
          WRITE(*,*) 'COMPNTS = ',COMPNTS
          WRITE(*,*) 'ERROR(RALEIGH) - GEOMERR must be positive'
          LERROR=.TRUE.
        END IF
        IF (LERROR) THEN
          LFAIL=.TRUE.
          WRITE(*,*) 'Error(s) found in the main parameters of RALEIGH'
          WRITE(*,*) 'Execution terminated'
          STOP
        END IF
      END IF

C Find the diameter DIAM of the plate
      DIAM=0.0
      DO 100 IV=1,GNUM-1
        PA(1)=GRID(IV,1)
        PA(2)=GRID(IV,2)
        PA(3)=GRID(IV,3)
        DO 110 JV=IV+1,GNUM
          PB(1)=GRID(JV,1)
          PB(2)=GRID(JV,2)
          PB(3)=GRID(JV,3)
          DIAM=MAX(DIAM,DIST3(PA,PB))
110     CONTINUE
100   CONTINUE

      IF (VALIDFG) THEN
        LERROR=.FALSE.


C Check that GEOMERR is not too large
        IF (GEOMERR.GT.DIAM/100.0D0) THEN
          WRITE(*,*) 'GEOMERR = ',GEOMERR
          WRITE(*,*) 'ERROR(RALEIGH) - GEOMERR is set too large'
          LERROR=.TRUE.
        END IF
        IF (LERROR) THEN
          LFAIL=.TRUE.
          WRITE(*,*)
          WRITE(*,*) 'Error in plate geometry or GEOMERR'
          WRITE(*,*) 'Execution terminated'
        END IF
      END IF                  

      IF (VALIDFG) THEN
C Check that the vertices are distinct with respect to GEOMERR
        LERROR=.FALSE.
        DO 130 IV=1,GNUM-1
          PA(1)=GRID(IV,1)
          PA(2)=GRID(IV,2)
          PA(3)=GRID(IV,3)
          DO 140 JV=IV+1,GNUM
            PB(1)=GRID(JV,1)
            PB(2)=GRID(JV,2)
            PB(3)=GRID(JV,3)
            IF (ABS(PA(1)-PB(1)).LT.GEOMERR) THEN
              IF (ABS(PA(2)-PB(2)).LT.GEOMERR) THEN
                IF (ABS(PA(3)-PB(3)).LT.GEOMERR) THEN
                  WRITE(*,*) 'Vertices ',IV,JV,' are not distinct'
                  LERROR=.TRUE.
                END IF
              END IF
            END IF
140       CONTINUE
130     CONTINUE
        IF (LERROR) THEN
          WRITE(*,*) 
          WRITE(*,*) 'ERROR(RALEIGH) - Vertices (see above) coincide'
          WRITE(*,*) 'Execution terminated'
          STOP
        END IF
      END IF          

C Check that vertices are co-planar
      LERROR=.FALSE.
C  Find the normal to the first element
      PA(1)=GRID(TRIA3(1,1),1)
      PA(2)=GRID(TRIA3(1,1),2)
      PA(3)=GRID(TRIA3(1,1),3)
      PB(1)=GRID(TRIA3(1,2),1)
      PB(2)=GRID(TRIA3(1,2),2)
      PB(3)=GRID(TRIA3(1,2),3)
      PC(1)=GRID(TRIA3(1,3),1)
      PC(2)=GRID(TRIA3(1,3),2)
      PC(3)=GRID(TRIA3(1,3),3)
      CALL NORM3(PA,PB,PC,NORMP)
C  Find the normals to each of the other elements
      DO 125 II=2,NELM
        QA(1)=GRID(TRIA3(II,1),1)
        QA(2)=GRID(TRIA3(II,1),2)
        QA(3)=GRID(TRIA3(II,1),3)
        QB(1)=GRID(TRIA3(II,2),1)
        QB(2)=GRID(TRIA3(II,2),2)
        QB(3)=GRID(TRIA3(II,2),3)
        QC(1)=GRID(TRIA3(II,3),1)
        QC(2)=GRID(TRIA3(II,3),2)
        QC(3)=GRID(TRIA3(II,3),3)
        CALL NORM3(QA,QB,QC,NORMQ)
        IF (DIST3(NORMP,NORMQ).GT.GEOMERR) THEN
          LERROR=.TRUE.
        END IF
125   CONTINUE
      IF (LERROR) THEN
          WRITE(*,*) 'WARNING(RALEIGH) - Panels are not co-planar'
      END IF
C Check that the elements are not of disproportionate sizes
      IF (VALIDFG) THEN
        SIZMAX=ZERO
        SIZMIN=DIAM**2
        DO 150 II=1,NELM
          QA(1)=GRID(TRIA3(II,1),1)
          QA(2)=GRID(TRIA3(II,1),2)
          QA(3)=GRID(TRIA3(II,1),3)
          QB(1)=GRID(TRIA3(II,2),1)
          QB(2)=GRID(TRIA3(II,2),2)
          QB(3)=GRID(TRIA3(II,2),3)
          QC(1)=GRID(TRIA3(II,3),1)
          QC(2)=GRID(TRIA3(II,3),2)
          QC(3)=GRID(TRIA3(II,3),3)
          SIZE=AREA(QA,QB,QC)
          SIZMAX=MAX(SIZMAX,SIZE)
          SIZMIN=MIN(SIZMIN,SIZE)
150     CONTINUE
        IF (SIZMAX.GT.10.0D0*SIZMIN) THEN
          WRITE(*,*) 'WARNING(RALEIGH) - Elements of disproportionate'
          WRITE(*,*) ' sizes'
        END IF
      END IF          
          

C Validation of the plate functions
      IF (VALIDFG) THEN
        LERROR=.FALSE.
        DO 170 II=1,NELM
          IF (MAX(ABS(ALPHA(II)),ABS(BETA(II))).LT.1.0D-6) 
     *     LERROR=.TRUE.
170     CONTINUE
        IF (LERROR) THEN
          WRITE(*,*) 
          WRITE(*,*) 'ERROR(RALEIGH) - at most one of ALPHA(i),BETA(i)'
          WRITE(*,*) ' may be zero for all i'
          WRITE(*,*) 'Execution terminated'
          STOP
        END IF
      END IF
        

C Set the wavenumber in complex form
      CK=CMPLX(Wavenum,ZERO)


C Set up validation and control parameters
C  Switch off the validation of HELMDIF
      LVAL=.FALSE.
C  Set EK
      EK=1.0D-6
C  Set EQRULE
      EQRULE=1.0D-6

C Set up the quadrature rule(s).
C  Set up quadrature rule for the case when P is not on the element.
C   Set up 8 point Gauss-Legendre rules
      CALL GAUSS(MAXNQ,NQOFF,WQOFF,XQOFF,YQOFF)
C  Set up quadrature rule for the case when P is on the element.

C   Set up quadrature rule data. If LPONEL is false then use the standard
C    Gaussian quadrature rule above. If LPONEL is true then then a
C    quadrature rule with 3 times as many points is used, this is made
C    up from three standard quadrature rules with the quadrature points
C    translated to the three triangles that each have the cetroid and two
C    of the original vertices as its vertices.
      NQON=3*NQOFF
      DO 330 IQ=1,NQOFF
        XQON(IQ)=XQOFF(IQ)*THIRD+YQOFF(IQ)
        YQON(IQ)=XQOFF(IQ)*THIRD
        WQON(IQ)=WQOFF(IQ)/THREE
        XQON(IQ+NQOFF)=XQOFF(IQ)*THIRD
        YQON(IQ+NQOFF)=XQOFF(IQ)*THIRD+YQOFF(IQ)
        WQON(IQ+NQOFF)=WQOFF(IQ)/THREE
        XQON(IQ+2*NQOFF)=THIRD*(ONE+TWO*XQOFF(IQ)-YQOFF(IQ))
        YQON(IQ+2*NQOFF)=THIRD*(ONE-XQOFF(IQ)+TWO*YQOFF(IQ))
        WQON(IQ+2*NQOFF)=WQOFF(IQ)/THREE
330   CONTINUE


C  Compute the discrete Lk, Mk, Mkt and Nk matrices
C   Loop(II) through the points on the plate
      DO 510 II=1,NELM
C    Set P
        PA(1)=GRID(TRIA3(II,1),1)
        PA(2)=GRID(TRIA3(II,1),2)
        PA(3)=GRID(TRIA3(II,1),3)
        PB(1)=GRID(TRIA3(II,2),1)
        PB(2)=GRID(TRIA3(II,2),2)
        PB(3)=GRID(TRIA3(II,2),3)
        PC(1)=GRID(TRIA3(II,3),1)
        PC(2)=GRID(TRIA3(II,3),2)
        PC(3)=GRID(TRIA3(II,3),3)
        P(1)=(PA(1)+PB(1)+PC(1))/THREE
        P(2)=(PA(2)+PB(2)+PC(2))/THREE
        P(3)=(PA(3)+PB(3)+PC(3))/THREE
C    Set VECP to the normal on the plate of the element at P
        CALL NORM3(PA,PB,PC,VECP)
C    Loop(II) through the elements
        DO 520 JJ=1,NELM
C     Set QA and QB, the coordinates of the edges of the ISEth element
          QA(1)=GRID(TRIA3(JJ,1),1)
          QA(2)=GRID(TRIA3(JJ,1),2)
          QA(3)=GRID(TRIA3(JJ,1),3)
          QB(1)=GRID(TRIA3(JJ,2),1)
          QB(2)=GRID(TRIA3(JJ,2),2)
          QB(3)=GRID(TRIA3(JJ,2),3)
          QC(1)=GRID(TRIA3(JJ,3),1)
          QC(2)=GRID(TRIA3(JJ,3),2)
          QC(3)=GRID(TRIA3(JJ,3),3)

C     Set LPONEL
          IF (II.EQ.JJ) THEN
            LPONEL=.TRUE.
          ELSE
            LPONEL=.FALSE.
          END IF

C     Select quadrature rule for HELMDIF
C   :  Select the quadrature rule XQON-WQON in the case when the
C   :   point p lies on the element, otherwise select XQOFF-WQOFF
C      [Note that the overall method would benefit from selecting from
C       a wider set of quadrature rules, and an appropriate method
C       of selection]
          IF (LPONEL) THEN
            NQ=NQON
            DO 600 IQ=1,NQ
              XQ(IQ)=XQON(IQ)
              YQ(IQ)=YQON(IQ)
              WQ(IQ)=WQON(IQ)
600         CONTINUE
          ELSE 
            NQ=NQOFF
            DO 610 IQ=1,NQ
              XQ(IQ)=XQOFF(IQ)
              YQ(IQ)=YQOFF(IQ)
              WQ(IQ)=WQOFF(IQ)
610         CONTINUE
          END IF

C   All operators are required
          LLK=.TRUE.
          LMK=.FALSE.
          LMKT=.FALSE.
          LNK=.FALSE.

C     Call HELMDIF.
          CALL HELMDIF(CK,P,VECP,QA,QB,QC,LPONEL,
     *     MAXNQ,NQ,XQ,YQ,WQ,
     *     LVAL,EK,GEOMERR,EQRULE,LFAIL1,
     *     LLK,LMK,LMKT,LNK,DISLK,DISMK,DISMKT,DISNK)

          WORK1(II,JJ)=-2.0D0*DISLK
          WORK3(II,JJ)=CZERO
         
C    Close loop(JJ) 
520     CONTINUE

        WORK3(II,II)=CONE
        WORK5(II)=CZERO
            
C   Close loop(II) 
510   CONTINUE

      IF (SOLFG) THEN
       CALL LinearSolver(MAXNELM,NELM,WORK3,WORK1,WORK5,ALPHA,BETA,
     *       FUNFV,PHI_PLATE,VEL_PLATE,LFAIL,WORK4,WORKFLG)        
      END IF  

C  SOLUTION IN THE DOMAIN

C   Compute sound pressures at the selected exterior points.
C    Loop through the the points in the exterior region
      DO 800 KK=1,COMPNTS
C    Set P
        P(1)=CORD_PNTS(KK,1)
        P(2)=CORD_PNTS(KK,2)
        P(3)=CORD_PNTS(KK,3)
C    Set VECP, this is arbitrary as the velocity/intensity at P
C     is not sought.
        VECP(1)=ONE
        VECP(2)=ZERO
        VECP(3)=ZERO

C    Initialise SUMPHI to zero
        SUMPHI=CZERO

C    Loop(II) through the elements
        DO 850 JJ=1,NELM
C     Compute the discrete Lk and Mk integral operators. 
            
C     Set QA and QB, the coordinates of the edges of the ISEth element
          QA(1)=GRID(TRIA3(JJ,1),1)
          QA(2)=GRID(TRIA3(JJ,1),2)
          QA(3)=GRID(TRIA3(JJ,1),3)
          QB(1)=GRID(TRIA3(JJ,2),1)
          QB(2)=GRID(TRIA3(JJ,2),2)
          QB(3)=GRID(TRIA3(JJ,2),3)
          QC(1)=GRID(TRIA3(JJ,3),1)
          QC(2)=GRID(TRIA3(JJ,3),2)
          QC(3)=GRID(TRIA3(JJ,3),3)

C     All the points do not lie on the plate hence LPONEL=.FALSE.
          LPONEL=.FALSE.              

C     Only Lk, Mk operators are required. Set LLK,LMK true, 
C      LMKT,LNK false. 
          LLK=.TRUE.
          LMK=.FALSE.
          LMKT=.FALSE.
          LNK=.FALSE.
                

C     Call HELMDIF.
          CALL HELMDIF(CK,P,VECP,QA,QB,QC,LPONEL,
     *     MAXNQ,NQOFF,XQOFF,YQOFF,WQOFF,
     *     LVAL,EK,GEOMERR,EQRULE,LFAIL,
     *     LLK,LMK,LMKT,LNK,DISLK,DISMK,DISMKT,DISNK)

C     Accumulate phi 
          SUMPHI=SUMPHI-2.0D0*DISLK*VEL_PLATE(JJ)
          IF (SOLFG) WORK2(KK,JJ)=-2.0D0*DISLK

C      Close loop (JJ) through the elements
850     CONTINUE

        PHI_EXT(KK)=SUMPHI

C     Close loop(KK) through the exterior points
800   CONTINUE

      END


C  Set up standard Gaussian Quadrature rule. Stores the values
C   for the 7 point Gaussian quadrature over the standard triangle.
   
      SUBROUTINE GAUSS(MAXNQ,NQ,WQ,XQ,YQ)
C      
        INTEGER MAXNQ,NQ
        REAL*8 WQ(MAXNQ),XQ(MAXNQ),YQ(MAXNQ)
        NQ=7
        WQ(1)=0.225000000000000D0
        WQ(2)=0.125939180544827D0
        WQ(3)=0.125939180544827D0
        WQ(4)=0.125939180544827D0
        WQ(5)=0.132394152788506D0
        WQ(6)=0.132394152788506D0
        WQ(7)=0.132394152788506D0

        XQ(1)=0.333333333333333D0
        XQ(2)=0.797426985353087D0
        XQ(3)=0.101286507323456D0
        XQ(4)=0.101286507323456D0
        XQ(5)=0.470142064105115D0
        XQ(6)=0.470142064105115D0
        XQ(7)=0.059715871789770D0

        YQ(1)=0.333333333333333D0
        YQ(2)=0.101286507323456D0
        YQ(3)=0.797426985353087D0
        YQ(4)=0.101286507323456D0
        YQ(5)=0.470142064105115D0
        YQ(6)=0.059715871789770D0
        YQ(7)=0.470142064105115D0
      END