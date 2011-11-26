      PROGRAM READEXCIT
C
C     THE PURPOSE OF THIS ROUTINE IS TO CONVERT EXCITING LOADS IN
C     VSAR PROGRAMS INTO INPUT LOADS FOR NX NASTRAN TO EXECUTE MODAL
C     TRANSIENT ANALYSIS FOR LOWER STRUCTURE (RAILS,BRIDGES OR TUNNELS).
C     
C     INPUT FILES  - b1~bn.txt
C     OUTPUT FILES - force.dat,moment.dat,dload.dat
C 
C     By Wenwei Chen
C
C
C     DEFINE VARIABLES
C
      IMPLICIT NONE
C**************************************
C*   Descriptions for Main Variables  *
C**************************************
C    INODE  - Node number
C    ICOMP  - Component number (1-6) of a node
C    IUN    - Input file
C    IOUT1  - Output1 file
C    IOUT2  - Output2 file
C    DTS    - Time step increment(Delta time step,default is 5.0E-04)
C    TMSTEP - Time step
C    VFORCE - Vertical Force component
C    BMOMENT- Bending moment
C    TSF()  - The array for time series of vertical force
C    TSM()  - The array for time series of bending moment
C    VF()   - The array for vertical force corresponding to TSF()
C    BM()   - The array for bending moment corresponding to TSM()
C    ILOOP  - Counter for whole loops, basically one loop per file
C    ISHFT1 - Used for define ID numbers(ISHFT1+ILOOP) for TABLED1&DAREA cards for VFORCE
C    ISHFT2 - Used for define ID numbers(ISHFT2+ILOOP) for TABLED1&DAREA cards for BMOMENT
C    ICOUNT - Used for define SID numbers for TLOAD1 cards, which included by DLOAD card
C  
      INTEGER  INODE,ICOMP,IUN,IOUT1,IOUT2,SYSIN,SYSOUT
	REAL     DTS,TMSTEP,VFORCE,BMOMENT, TINY
	REAL     TSF(1000),TSM(1000),VF(1000),BM(1000)
C
	INTEGER  II,JJ,KK,NN,ILOOP,ICOUNT,ISHFT1,ISHFT2,IRET
	CHARACTER*72 INPUT,OUTPUT1,OUTPUT2,TEMP
	DATA SYSIN /5/
      DATA SYSOUT /6/
	DATA ISHFT1,ISHFT2 /1000,2000/
	DATA TINY /1.0E-06/
C
C     initialize
C
      INODE = 0
	ILOOP = 0
	ICOUNT = 0
	DTS = 0.5E-03
	TMSTEP = 0.0
	VFORCE = 0.0
	BMOMENT = 0.0
	II = 0
	JJ = 0
	KK = 0
	NN = 0
C
	DO II = 1, 1000
	   TSF(II) = 0.0
	   TSM(II) = 0.0
	   VF(II) = 0.0
	   BM(II) = 0.0
	ENDDO    
C
      IUN = 21
	IOUT1 = 22
	IOUT2 = 23
C
C     OPEN FILES & WRITE LOADS FILES
C
      OUTPUT1 = 'force.dat'
	OUTPUT2 = 'moment.dat'
C
      OPEN (UNIT=IOUT1,FILE=OUTPUT1,STATUS='UNKNOWN',ERR=99,IOSTAT=IRET)
	OPEN (UNIT=IOUT2,FILE=OUTPUT2,STATUS='UNKNOWN',ERR=99,IOSTAT=IRET)
C       
   10 ILOOP = ILOOP + 1
      WRITE( TEMP,'(I5)' ) ILOOP
C     Generate bn.txt for open and read
	INPUT = 'b'//trim(adjustl(TEMP))//'.txt'
C
      OPEN (UNIT=IUN,FILE=INPUT,STATUS='OLD',ERR=90,IOSTAT=IRET)
C     
      REWIND (IUN)
C
C     INITIAL TIME POINT - ZERO VALUES FOR FORCE & MOMENT
C
      JJ = JJ + 1
      KK = KK + 1
      TSF(1) = 0.0
	TSM(1) = 0.0
	VF(1) = 0.0
	BM(1) = 0.0
C
   15	CONTINUE
C
C     Read the n-th input file bn.txt,if meet end of the file,
C     then goto 25 for writing TABLED1,DAREA,TLOAD1 cards for this file
C
	READ(IUN,20,END=25) TMSTEP,INODE,VFORCE,BMOMENT
   20 FORMAT(2X,1pe15.6,I10,3X,1pe15.6,3X,1pe15.6)

C
C     Generally, the first time point is not equal to 0.5E-03,so we must
C     dertemine how long the zero values take,or it will linearly grow from
C     zero values to the current time point. The interim time points also meet
C     such case. The illustration as below describes such case.
C
C     Notice: The * path should be correct for such load excitation
C     Value         *
C     |         #  *
C     |      #    *
C     |    #     *
C     |  #      *
C     |--* * * *-----------> t
C
      IF (JJ .EQ. 1) THEN     
         IF (TMSTEP.GT.DTS*1.5) THEN
            JJ = JJ + 2
	      KK = KK + 2
C
	      TSF(JJ-1) = TMSTEP -DTS
	      VF(JJ-1) = 0.0
	      TSF(JJ) = TMSTEP
            VF(JJ) = VFORCE
C
	      TSM(KK-1) = TMSTEP -DTS
	      BM(KK-1) = 0.0
		  TSM(KK) = TMSTEP
	      BM(KK) = BMOMENT
	   ELSE
	      JJ = JJ + 1
	      KK = KK + 1
C
	      TSF(JJ) = TMSTEP
            VF(JJ) = VFORCE
C
		  TSM(KK) = TMSTEP
	      BM(KK) = BMOMENT
	   ENDIF
      ELSE
C
C     Most of zero data pairs will be removed in order to 
C     save memory, only first and last time point of zero
C     -value timeline kept for the arrays.
C
C     FORCE
C
	   IF (VFORCE .GT. TINY) THEN
	      IF ((TMSTEP-TSF(JJ)).GT.DTS*1.5) THEN
	          JJ = JJ + 2
	          TSF(JJ-1) = TMSTEP -DTS
	          VF(JJ-1) = 0.0
	          TSF(JJ) = TMSTEP
                VF(JJ) = VFORCE
	      ELSE
	          JJ = JJ + 1
	          TSF(JJ) = TMSTEP
                VF(JJ) = VFORCE
	      ENDIF
         ENDIF
C
C     MOMENT
C
         IF (BMOMENT .GT. TINY) THEN
	      IF ((TMSTEP-TSM(KK)).GT.DTS*1.5) THEN
	          KK = KK + 2
	          TSM(KK-1) = TMSTEP -DTS
	          BM(KK-1) = 0.0
	          TSM(KK) = TMSTEP
                BM(KK) = BMOMENT
	      ELSE
	          KK = KK + 1
	          TSM(KK) = TMSTEP
                BM(KK) = BMOMENT
	      ENDIF
         ENDIF
      ENDIF
C
      GO TO 15
C
   25 CONTINUE
C
      CLOSE(IUN)
C
C     CORRECT THE LAST DATA PAIRS
C
      JJ = JJ + 1
	KK = KK + 1
	TSF(JJ) = TSF(JJ-1) + DTS
	VF(JJ) = 0.0
	TSM(KK) = TSM(KK-1) + DTS
	BM(KK) = 0.0
C
C     FIRSTLY WRITE OUT TABLED1 CARD HEADER
C
      WRITE(IOUT1,28)
	WRITE(IOUT2,28)
   28 FORMAT('$******************************************************$')
      WRITE(IOUT1,30) ISHFT1 + ILOOP
      WRITE(IOUT2,30) ISHFT2 + ILOOP
   30 FORMAT('TABLED1 ',I8)
C     **************************
C     WRITE DATA PAIRS FOR FORCE
C     **************************
C     Four data pairs per continuation lines
      NN = JJ/4
	JJ = MOD(JJ,4)
	DO II = 1, 4-JJ
	  TSF(4*NN+JJ+II)=TSF(4*NN+JJ)+II*DTS
      ENDDO
C
	DO II=1, NN+1
	   WRITE(IOUT1,35) TSF(4*II-3),VF(4*II-3),TSF(4*II-2),VF(4*II-2),
     1                   TSF(4*II-1),VF(4*II-1),TSF(4*II),VF(4*II)
      ENDDO
C	 
   35 FORMAT(8X,4(F8.6,F8.1))
C     THEN WRITE THE LAST LINE
      WRITE(IOUT1,40) 
   40 FORMAT(8X,'    ENDT')
      WRITE(IOUT1,28)
C
C     THEN WRITE TLOAD1 & DAREA CARDS
C
      ICOMP = 3
      WRITE(IOUT1,45) ISHFT1+ILOOP,INODE,ICOMP
   45 FORMAT('DAREA   ',3I8,'     1.0')
C
      WRITE(IOUT1,28)
C
      ICOUNT = ICOUNT + 1
      WRITE(IOUT1,50) ICOUNT,ISHFT1+ILOOP,ISHFT1+ILOOP
   50 FORMAT('TLOAD1  ',2I8,2('        '),I8)
C
      WRITE(IOUT1,28)
C     **************************
C     WRITE DATA PAIRS FOR MOMENT
C     **************************
      NN = KK/4
	KK = MOD(KK,4)
	DO II = 1, 4-KK
	  TSM(4*NN+KK+II)=TSM(4*NN+KK)+II*DTS
      ENDDO
C
	DO II=1, NN
	   WRITE(IOUT2,35) TSM(4*II-3),BM(4*II-3),TSM(4*II-2),BM(4*II-2),
     1                   TSM(4*II-1),BM(4*II-1),TSM(4*II),BM(4*II)
      ENDDO
C	 
C     THEN WRITE THE LAST LINE
      WRITE(IOUT2,40)
      WRITE(IOUT2,28)
C
C     THEN WRITE TLOAD1 & DAREA CARDS
C
      ICOMP = 4
      WRITE(IOUT2,45) ISHFT2+ILOOP,INODE,ICOMP
C
      WRITE(IOUT2,28)
C
      ICOUNT = ICOUNT + 1
      WRITE(IOUT2,50) ICOUNT,ISHFT2+ILOOP,ISHFT2+ILOOP
C
      WRITE(IOUT2,28)
C
C     If finishing the current file, then initializing the 
C     related variables and arrays and GO TO 10 to deal with
C     the next file
C 
      CONTINUE
	II = 0
	JJ = 0
	KK = 0
	NN = 0
C
	DO II = 1, 1000
	   TSF(II) = 0.0
	   TSM(II) = 0.0
	   VF(II) = 0.0
	   BM(II) = 0.0
	ENDDO
	GO TO 10
C
C*********************************************************************
C Finished dealing with all input files and writing out force.dat    *
C and moment.dat, then starting to write out dload.dat(define DLOAD) *
C ,which will contain all SID of TLOAD1 in the force.dat & moment.dat*
C*********************************************************************
   90 WRITE (SYSOUT,*) 'TOTALLY',ILOOP-1,'FILES ARE TRANSLATED.'
      CLOSE(IOUT1)
	CLOSE(IOUT2)
C
C     WRITE OUT DLOAD.DAT
C
      II = 0
      OUTPUT1 = 'dload.dat'
C
C    The first line of DLOAD could contain 3 data pairs, then 4 data pairs
C    for per continuation lines. And the scale factors are all 1.0.
C
      OPEN (UNIT=IOUT1,FILE=OUTPUT1,STATUS='UNKNOWN',ERR=99,IOSTAT=IRET)
C     Comment line
	WRITE(IOUT1,28)
C
      NN = (ICOUNT-3)/4
	KK = MOD(ICOUNT-3,4)
C
C     Write out the first line for DLOAD
C     Special cases for ICOUNT=1,2
C
      IF(ICOUNT .EQ. 1) THEN
		WRITE(IOUT1,53) II+1
      ELSEIF(ICOUNT .EQ. 2) THEN
          WRITE(IOUT1,54) II+1,II+2
	ELSE
	    WRITE(IOUT1,55) II+1,II+2,II+3
      ENDIF
C
   53 FORMAT('DLOAD        100     1.0     1.0',I8)
   54 FORMAT('DLOAD        100     1.0     1.0',I8,'     1.0',I8)
   55 FORMAT('DLOAD        100     1.0     1.0',I8,2('     1.0',I8))
C
C     Write out continuation lines for DLOAD
C
      DO II=1, NN
	   WRITE(IOUT1,60) 4*(II-1)+4,4*(II-1)+5,4*(II-1)+6,4*(II-1)+7
	ENDDO
   60 FORMAT(8X,4('     1.0',I8))
C
C     WRITE THE LAST LINE FOR DLOAD CARD
C
      IF (KK .EQ. 1) THEN
	   WRITE(IOUT1,61) ICOUNT
   61    FORMAT(8X,'     1.0',I8)
      ELSEIF(KK .EQ. 2) THEN
	   WRITE(IOUT1,62) ICOUNT-1,ICOUNT
   62    FORMAT(8X,2('     1.0',I8))
      ELSEIF(KK .EQ. 3) THEN
	   WRITE(IOUT1,63) ICOUNT-2,ICOUNT-1,ICOUNT
   63    FORMAT(8X,3('     1.0',I8))
      ENDIF
C
      CLOSE(IOUT1)
C				      				 
      GO TO 100
C
   99 WRITE (SYSOUT,*) 'OPEN ERROR FOR OUTPUT FILE, IOSTAT=',IRET
C
  100 CONTINUE
      END