C 
C  INTERNAL FUNCTIONS FOR ROUTINES
C
       REAL*8 FUNCTION FNSQRT(X)
       REAL*8 X
       FNSQRT=SQRT(X)
       END

C  Subroutine for returning the exponential.
       COMPLEX*16 FUNCTION FNEXP(Z)
       COMPLEX*16 Z
       FNEXP=EXP(Z)
       END                                            
C complex function ICMULT: Returns the result of multiplying a complex
C  number by i.
      COMPLEX*16 FUNCTION ICMULT(Z)
      COMPLEX*16 Z
      REAL*8 REZ,IMZ
      REZ=DBLE(Z)
      IMZ=DIMAG(Z)
      ICMULT=CMPLX(-IMZ,REZ)
      END

C real function DIST3: Returns the distance between two 3-vectors.
C Requires the external module SIZE3, SUBV3 (supplied).
      REAL*8 FUNCTION DIST3(VECA,VECB)
      REAL*8 VECA(3),VECB(3),SIZE3
      REAL*8 VEC(3)
      CALL SUBV3(VECA,VECB,VEC)
      DIST3=SIZE3(VEC)
      END

C real function AREA: Returns the area of the triangle with the vertices
C  of three 3-vectors. Requires the external modules SSIZE3, DOT3 and
C  SUBV3 (supplied).
      REAL*8 FUNCTION AREA(VECA,VECB,VECC)
      REAL*8 VECA(3),VECB(3),VECC(3)
      REAL*8 RQAQB,RQAQC,VECD(3),VECE(3),DP
      REAL*8 SSIZE3,DOT3
      CALL SUBV3(VECB,VECA,VECD)
      CALL SUBV3(VECC,VECA,VECE)
      RQAQB=SSIZE3(VECD)
      RQAQC=SSIZE3(VECE)
      DP=DOT3(VECD,VECE)
      AREA=SQRT(RQAQB*RQAQC-DP*DP)/2.0D0
      END

C real function SSIZE3: Returns the square of the modulus of a 3-vector.
C Requires the external module DOT3 (supplied).
      REAL*8 FUNCTION SSIZE3(VEC)
      REAL*8 VEC(3),DOT3
      SSIZE3=DOT3(VEC,VEC)
      END

C real function SIZE3: Returns the modulus of a 3-vector.
C Requires the extrnal module SSIZE3 (supplied).
      REAL*8 FUNCTION SIZE3(VEC)
      REAL*8 VEC(3),SSIZE3
      SIZE3=SQRT(SSIZE3(VEC))
      END

C real function DOT3: Returns the dot product of two 3-vectors.
      REAL*8 FUNCTION DOT3(VECA,VECB)
      REAL*8 VECA(3),VECB(3)
      DOT3=VECA(1)*VECB(1)+VECA(2)*VECB(2)+VECA(3)*VECB(3)
      END

C subroutine SUBV3: Gives the output in VEC for the subtraction
C  of 3-vector VECB from VECA.
      SUBROUTINE SUBV3(VECA,VECB,VEC)
      REAL*8 VECA(3),VECB(3),VEC(3)
      VEC(1)=VECA(1)-VECB(1)
      VEC(2)=VECA(2)-VECB(2)
      VEC(3)=VECA(3)-VECB(3)
      END 

C subroutine NORM3: Gives the output in VECNOR for the unit normal 
C  to the plane containing the 3-vectors VECA, VECB and VECC
      SUBROUTINE NORM3(VECA,VECB,VECC,VECNOR)
      REAL*8 VECA(3),VECB(3),VECC(3),VECNOR(3)
      REAL*8 VBMVA(3),VCMVA(3)
      REAL*8 CR1,CR2,CR3,SCR,ACR
      CALL SUBV3(VECB,VECA,VBMVA)
      CALL SUBV3(VECC,VECA,VCMVA)
      CR1=VBMVA(2)*VCMVA(3)-VBMVA(3)*VCMVA(2)
      CR2=VCMVA(1)*VBMVA(3)-VBMVA(1)*VCMVA(3)
      CR3=VBMVA(1)*VCMVA(2)-VBMVA(2)*VCMVA(1)
      SCR=CR1*CR1+CR2*CR2+CR3*CR3
      ACR=SQRT(SCR)
      VECNOR(1)=CR1/ACR
      VECNOR(2)=CR2/ACR
      VECNOR(3)=CR3/ACR
      END
                                                                                                     