      Subroutine ReadVelo (Freq,Real_Part,Imag_Part,SampNum,INODE)
C
C     THE PURPOSE OF THIS ROUTINE IS TO READ NODAL VELOCITY
C 
C     By Wenwei Chen
C
C
C     DEFINE VARIABLES
C
      IMPLICIT NONE
C   
      INTEGER  SampNum,INODE,ISHFT,IUN,SYSIN,SYSOUT
	Double Precision  Freq(SampNum),Real_Part(SampNum,INODE),
     1	              Imag_Part(SampNum,INODE)
	Double Precision  FREQV,REALV,IMAGV
C
      LOGICAL  OpenFlag
	INTEGER  II, JJ, KK, NN, ILOOP, ICOUNT, IRET
	CHARACTER*80 INPUT,TEMP,LINE,FRQ,RL,IM
	DATA SYSIN /5/
      DATA SYSOUT /6/
	DATA ISHFT /21/
C
C     initialize
C
      ILOOP = 0
	ICOUNT= 0
	II = 0
	JJ = 0
	KK = 0
	NN = 0
      OpenFlag = .FALSE.
C
      DO II = 1, SampNum
         Freq(II) = 0.0D0
	ENDDO
C
	DO II = 1, INODE
	   DO JJ = 1, SampNum     
	     Real_Part(JJ,II) = 0.0D0
	     Imag_Part(JJ,II) = 0.0D0
	   ENDDO
	ENDDO    
C
      IUN = 21
C
C     OPEN FILES
C       
   10 ILOOP = ILOOP + 1
      WRITE( TEMP,'(I5)' ) ILOOP
	INPUT = 'node'//trim(adjustl(TEMP))//'.txt'
C
      OPEN (UNIT=IUN,FILE=INPUT,STATUS='OLD',ERR=90,IOSTAT=IRET)
      OpenFlag = .TRUE.
C     
      REWIND (IUN)
C
   15	CONTINUE
C    READ DATA LINE BY LINE
      DO II=1, SampNum + ISHFT
C     Read the whole line
	  READ(IUN,'(A)') LINE
  	  TEMP=trim(LINE)
C     DATA LINE
        IF(TEMP(7:11).EQ.'.    ') THEN
C
         ICOUNT = ICOUNT + 1
C        
         FRQ = TEMP(12:24)
         FRQ = trim(adjustl(FRQ))
         READ(FRQ,'(1pD15.6)') FREQV
C
	     RL  = TEMP(29:42)
	     RL  = trim(adjustl(RL))
	     READ(RL,'(1pD15.6)') REALV
C
	     IM  = TEMP(46:60)
	     IM  = trim(adjustl(IM))
	     READ(IM,'(1pD15.6)') IMAGV
C
C      Frequency Values are same for every point
C
         FREQ(ICOUNT) = FREQV       
         Real_Part(ICOUNT,ILOOP) = REALV
	     Imag_Part(ICOUNT,ILOOP) = IMAGV
        ENDIF
      ENDDO
C
      CONTINUE
	ICOUNT = 0
      OpenFlag=.FALSE.
      GO TO 10
C
   90 CONTINUE
      IF(ILOOP.EQ.1 .AND. .NOT.OpenFlag) THEN
         WRITE (SYSOUT,*) 'Fatal Error: Can not open the first file.' 
         STOP
      ENDIF    
      WRITE (SYSOUT,*) 'TOTALLY',ILOOP-1,'FILES ARE TRANSLATED.'
      GO TO 100    
C     
  100 CONTINUE
      END