!***********************************************************************
!                                                                      *
!  // define global variables //                                       *
!                                                                      *
!***********************************************************************
      module global

      real*8  M_C,Length_C,Lc_C,M_T,M_W,K_1,C_1,K_2,C_2,L_C,L_T

      real*8  E_B,Density_B,Are_B,J_B,LenB_E,Length_B
	
      real*8  Velocity,Delta_T,T_Max,Lw1,Lw2,Lw3,lw4,Gt

	real*8  E_R,Density_R,Are_R,J_R,LengthR_E,Length_R

!     节点刚度/阻尼，每一个板子的参数

	real*8  M_F,Density_F,lengthF_E,Length_F,K_RF,C_RF,K_BF,C_BF

!     K_r,C_r轨下面刚度/阻尼，每一个板子的参数在传递率特性研究中用到的,其中
!      uDensity_F,aK_BF,bC_BF分别代表u a b .

	real*8  uDensity_F,K_r,C_r,aK_BF,bC_BF,E_F,J_F

      integer Num_C,Num_B,Ne_R,Ne_B,Mnode_R,Mnode_B,Mdf_R,Mdf_B

      integer Nu_F,Ne_F,Mnode_F,Mdf_F,Num_RF,Mdf_L,Mdf_C,Mvar,N_BS


      common M_C,Length_C,Lc_C,M_T,M_W,K_1,C_1,K_2,C_2,L_C,L_T

      common E_B,Density_B,Are_B,J_B,LenB_E,Length_B

      common Velocity,Delta_T,T_Max,Lw1,Lw2,Lw3,lw4,Gt

	common E_R,Density_R,Are_R,J_R,LengthR_E,Length_R

	common M_F,Density_F,lengthF_E,Length_F,K_RF,C_RF,K_BF,C_BF

	common uDensity_F,K_r,C_r,aK_BF,bC_BF,E_F,J_F

      common Num_C,Num_B,Ne_R,Ne_B,Mnode_R,Mnode_B,Mdf_R,Mdf_B

	common Nu_F,Ne_F,Mnode_F,Mdf_F,Num_RF,Mdf_L,Mdf_C,Mvar,N_BS

      real*8,parameter::Pi=3.1415926,Gz=9.806,Gh=1.0e9,ep=1.0D-4

      end module

!***********************************************************************
!                                                                      *
!  // main Program //                                                  *
!                                                                      *
!***********************************************************************
      program VSAR

      use global

      real(8),allocatable::SM(:,:),Sk(:,:),SC(:,:)
	
      real(8),allocatable::MC(:,:),KC(:,:),CC(:,:)

      integer,allocatable::LnodBS(:,:),LnodRs(:,:),LnodFs(:,:)

      open(10,File='vehicle.dat',Status='unKnown')

      open(11,File='rail.dat',Status='unKnown')

      open(12,File='beam.dat',Status='unKnown')

      open(13,File='irr.dat',status='unknown')

      open(14,file='fslab.dat',status='unknown')

      open(15,File='calculation.dat',Status='unKnown')


      open(30,File='vehicle_out.dat',Status='unKnown')

      open(40,File='turn_out.dat',Status='unKnown')

      open(50,File='wheel_out.dat',Status='unKnown')

      open(60,File='forceR_OUt.dat',Status='unKnown')

! read the parameters of vehicle
      read(10,*)Num_C,M_C,Length_C,LC_C,M_T,M_W,K_1,C_1,K_2,C_2,L_C,L_T

! read the parameters of rail
      read(11,*)E_R,Density_R,Are_R,J_R,LengthR_E

! read the parameters of beam
!    一般取LenB_E=LengthF_E为LengthR_E的整数倍，方便计算。
      read(12,*)E_B,Density_B,Are_B,J_B

! read the parameters of fslab

!     N_BS是每一个浮置板单元上的支座个数。
      read(14,*)Nu_F,Length_F,LengthF_E,uDensity_F,K_R,C_R,aK_BF,
     $	bC_BF,E_F,J_F,N_BS

! read the parameters of calculations
      read(15,*)Velocity,Delta_T

! calculate the system degree of freedom
      call Mdf_cal()

! to form the top of beam
      allocate(LnodBS(Ne_B,2),LnodRS(Ne_R,2),LnodFS(Ne_F,2))
      call Top(LnodBS,LnodRS,LnodFS)

! to form the matrix of mass,stifness and damp
      allocate(SM(Mdf_L,Mdf_L),Sk(Mdf_L,Mdf_L),SC(Mdf_L,Mdf_L))
      call matrix_Low(LnodBS,LnodRS,LnodFS,SM,Sk,SC)

! to form the wheel's matrix of mass,stifness and damp
      allocate(MC(Mdf_C,Mdf_C),KC(Mdf_C,Mdf_C),CC(Mdf_C,Mdf_C))
      call Vehicle(MC,KC,CC)

! to calculate the dynamic reponse of the system
      call Steps(SM,Sk,SC,MC,KC,CC)

	end 

!***********************************************************************
!                                                                      *
! Subroutine to Calculate the system degree of freedom                 *
!                                                                      *
!***********************************************************************
      subroutine Mdf_cal()

      use global

      Mdf_C=10*Num_C


!     N_F是研究的轨道下所有的浮置板个数，NumR_F是每块板单元上的轨道单元数，

      Ne_R=Nu_F*int(Length_F/LengthR_E)
      Mnode_R=Ne_R+1
      Mdf_R=2*Mnode_R
    
      Num_RF=int(LengthF_E/LengthR_E)
!     Num_RF一般为了简单且计算准确，取值板单元长度为轨道单元长度的整数倍。

      Ne_F=Nu_F*int(Length_F/LengthF_E)
	Mnode_F=Ne_F+1
      Mdf_F=2*Mnode_F

      LenB_E=LengthF_E

      Ne_B=Ne_F
      Mnode_B=Ne_B+1
      Mdf_B=2*Mnode_B

      Mdf_L=Mdf_R+Mdf_B+Mdf_F

      Mvar=Mdf_C+Mdf_L

!总的轨道、以及梁长长度
      Length_R=Ne_R*LengthR_E
      Length_B=Length_R

!     ms=u*mr  同传递率方法中    
      Density_F=uDensity_F*Density_R
      Gt=(M_C+2*M_T+4*M_W)*Gz/4.0
	
!     将线节点刚度转换到mian刚度上  (在每一个浮置板上进行计算)  
  
      K_RF=k_r*length_F/(Num_RF*int(length_F/lengthF_E))
      C_RF=c_r*length_F/(Num_RF*int(length_F/lengthF_E))

!      k_r=K_RF*(Num_RF*int(length_F/lengthF_E))/length_F
!      c_r=C_RF*(Num_RF*int(length_F/lengthF_E))/length_F
	
	  

!     将支座刚度转换到节点刚度上 ， aK_BF、bc_BF相当于receptance程序中的a、b。
!     N_BS是每一个浮置板单元上的支座个数。
 
	K_BF=k_r*aK_BF*length_F/((N_BS-1)*int(length_F/lengthF_E)) 
	C_BF=c_r*bc_BF*length_F/((N_BS-1)*int(length_F/lengthF_E)) 

      

      lw1=0.0
      Lw2=2.0*L_T
      Lw3=2.0*L_C
      Lw4=2.0*(L_C+L_T)


      T_Max=(Length_R+Num_C*lw4+(Num_C-1)*Lc_C)/Velocity

!     车一直在桥上所用时间

!     T_Max=(Length_R-lw4*Num_C-(Num_C-1)*LC_C)/Velocity
	Num_B=2

      return
      end
!***********************************************************************
!                                                                      *
! Subroutine to get the top of beam                                    *
!                                                                      *
!***********************************************************************
      subroutine Top(LnodBS,LnodRS,LnodFS)

      use global

      integer,intent(out)::LnodBS(Ne_B,2),LnodRS(Ne_R,2),LnodFS(Ne_F,2)

      do Ielement=1,Ne_B
        do Inode=1,2
        LnodBs(Ielement,Inode)=(Ielement-1)+Inode
        end do
      end do

      do Ielement=1,Ne_R
      do Inode=1,2
      LnodRs(Ielement,Inode)=(Ielement-1)+Inode
      end do
      end do


      do Ielement=1,Ne_F
      do Inode=1,2
      LnodFs(Ielement,Inode)=(Ielement-1)+Inode
      end do
      end do

      return
      end
!***********************************************************************
!                                                                      *
! subroutine to form the total matrix of mass,stifness and damp        *
!                                                                      *
!***********************************************************************
      subroutine matrix_Low(LnodBS,LnodRS,LnodFS,SM,Sk,SC)

      use global

      integer,intent(in)::LnodBS(Ne_B,2),LnodRS(Ne_R,2),LnodFS(Ne_F,2)

      real(8),intent(out)::SM(Mdf_L,Mdf_L),Sk(Mdf_L,Mdf_L),
     &	SC(Mdf_L,Mdf_L)
	 
	integer Num_P00,Num_P01,Num_P02,Num_P10,Num_P11,Num_P12

	integer I_B1,I_s1,I_B2,I_s2


      real(8),allocatable::MR(:,:),KR(:,:),CR(:,:)
	real(8),allocatable::MF(:,:),KF(:,:),CF(:,:)
	real(8),allocatable::MB(:,:),KB(:,:),CB(:,:)

	real(8)  shape1,shape2,shape11,shape12

! let the matrix sm,sk and sc to be zero
   
      SM=0.0
      Sk=0.0
      SC=0.0
    
!  put the matrix of rail into the total matrix

      allocate(MR(Mdf_R,Mdf_R),KR(Mdf_R,Mdf_R),CR(Mdf_R,Mdf_R))

      call Rail(LnodRS,MR,KR,CR)

      do I=1,Mdf_R
       do J=1,Mdf_R
       SM(I,J)=SM(I,J)+MR(I,J)
       SK(I,J)=SK(I,J)+KR(I,J)
       SC(I,J)=SC(I,J)+CR(I,J)
       end do
      end do
      deallocate (MR,KR,CR)    
    
! put the matrix of fslab into the total matrix

      allocate (MF(Mdf_F,Mdf_F),KF(Mdf_F,Mdf_F),CF(Mdf_F,Mdf_F))

      call FSlab(LnodfS,MF,KF,CF)

      do I=Mdf_R+1,Mdf_R+Mdf_F
       do J=Mdf_R+1,Mdf_R+Mdf_F
       SM(I,J)=SM(I,J)+MF(I-Mdf_R,J-Mdf_R)
       SK(I,J)=SK(I,J)+KF(I-Mdf_R,J-Mdf_R)
       SC(I,J)=SC(I,J)+CF(I-Mdf_R,J-Mdf_R)
       end do
      end do 

      deallocate (MF,KF,CF)

! put the matrix of beam into the total matrix

      allocate (MB(Mdf_B,Mdf_B),KB(Mdf_B,Mdf_B),CB(Mdf_B,Mdf_B))

      call Beam(LnodBS,MB,KB,CB)

      do I=Mdf_R+Mdf_F+1,Mdf_R+Mdf_F+Mdf_B
        do J=Mdf_R+Mdf_F+1,Mdf_R+Mdf_F+Mdf_B
        SM(I,J)=SM(I,J)+MB(I-Mdf_R-Mdf_F,J-Mdf_R-Mdf_F)
        Sk(I,J)=Sk(I,J)+KB(I-Mdf_R-Mdf_F,J-Mdf_R-Mdf_F)
        SC(I,J)=SC(I,J)+CB(I-Mdf_R-Mdf_F,J-Mdf_R-Mdf_F)
        end do
      end do 

      deallocate (MB,KB,CB)

! calculate the coupled items of Rail and slab to the rail

!     每块板上的浮置板单元总数
      NF1_E=INT(LENGTH_f/LENGTHF_E)

!     对所有的浮置板循环
	dO NUF=1,NU_F

!     对每块浮置板上的单元循环,从每一个板在总的单元拓扑中的编号开始.
      do I=(NUF-1)*NF1_E+1,NUF*NF1_E

!     每一个板单元上第一个轨道节点编号,上一个单元最后一个节点编号,最后一个点编号
      if (I.eq.((NUF-1)*NF1_E+1)) then
	Num_P00=(I-1)*Num_RF+1
	else
	Num_P00=(I-1)*Num_RF+1+1
	end if
	Num_P01=(I-1)*Num_RF+1
	Num_P02=I*Num_RF+1
!     对所有的轨道单元自由度循环,就是第NUF个板上第I个单元上的轨道节点自由度循环
      do I_R=Num_P00*2-1,Num_P02*2-1,2
      SK(I_R,I_R)=SK(I_R,I_R)+K_RF
      SC(I_R,I_R)=SC(I_R,I_R)+C_RF

      shape1=(Num_RF-((I_R+1)/2-Num_P01))/Num_RF
	shape2=1-shape1

      SK(I_R,Mdf_R+2*I-1)=SK(I_R,Mdf_R+2*I-1)-K_RF*shape1
      SC(I_R,Mdf_R+2*I-1)=SC(I_R,Mdf_R+2*I-1)-C_RF*shape1
      SK(I_R,Mdf_R+2*I+1)=SK(I_R,Mdf_R+2*I+1)-K_RF*shape2
      SC(I_R,Mdf_R+2*I+1)=SC(I_R,Mdf_R+2*I+1)-C_RF*shape2
      end do 
      end do
      end do

! calculate the coupled items of rail and slab to the slab

!     对所有的浮置板循环
	dO NUF=1,NU_F
!     对每块浮置板上的单元循环,从每一个板在总的单元拓扑中 的编号开始.
      do I=(NUF-1)*NF1_E+1,NUF*NF1_E

!     每一个板单元上第一个轨道节点编号Num_P10,上一个单元最后一个节点编号Num_P11,
!     最后一个点编号Num_P12
      if (I.eq.((NUF-1)*NF1_E+1)) then
	Num_P10=(I-1)*Num_RF+1
	else
	Num_P10=(I-1)*Num_RF+1+1
	end if
	Num_P11=(I-1)*Num_RF+1
	Num_P12=I*Num_RF+1

!     对所有的轨道单元自由度循环,就是第NUF个板上第I个单元上的轨道节点自由度循环

      do I_R=Num_P10*2-1,Num_P12*2-1,2

      shape11=(Num_RF-((I_R+1)/2-Num_P11))/Num_RF
	shape12=1-shape11
!     轨道第I_R个自由度对浮置板的作用力，对应于浮置板的单元的两个自由度即两行对应轨道的列
      SK(Mdf_R+2*I-1,I_R)=SK(Mdf_R+2*I-1,I_R)-K_RF*shape11
      SC(Mdf_R+2*I-1,I_R)=SC(Mdf_R+2*I-1,I_R)-C_RF*shape11
      SK(Mdf_R+2*I+1,I_R)=SK(Mdf_R+2*I,I_R)-K_RF*shape12
      SC(Mdf_R+2*I+1,I_R)=SC(Mdf_R+2*I,I_R)-C_RF*shape12

!     轨道第I_R个自由度对浮置板的作用力，对应于浮置板的单元的第一个自由度
      SK(Mdf_R+2*I-1,Mdf_R+2*I-1)=SK(Mdf_R+2*I-1,Mdf_R+2*I-1)+
     $	K_RF*shape11**2
      SK(Mdf_R+2*I-1,Mdf_R+2*I+1)=SK(Mdf_R+2*I-1,Mdf_R+2*I+1)+
     $	K_RF*shape11*shape12
      SC(Mdf_R+2*I-1,Mdf_R+2*I-1)=SC(Mdf_R+2*I-1,Mdf_R+2*I-1)+
     $	C_RF*shape11**2
      SC(Mdf_R+2*I-1,Mdf_R+2*I+1)=SC(Mdf_R+2*I-1,Mdf_R+2*I+1)+
     $	C_RF*shape11*shape12

!     轨道第I_R个自由度对浮置板的作用力，对应于浮置板的单元的第二个自由度
      SK(Mdf_R+2*I+1,Mdf_R+2*I+1)=SK(Mdf_R+2*I+1,Mdf_R+2*I+1)+
     $	K_RF*shape12**2
      SK(Mdf_R+2*I+1,Mdf_R+2*I-1)=SK(Mdf_R+2*I+1,Mdf_R+2*I-1)+
     $	K_RF*shape11*shape12
      SC(Mdf_R+2*I+1,Mdf_R+2*I+1)=SC(Mdf_R+2*I+1,Mdf_R+2*I+1)+
     $	C_RF*shape12**2
      SC(Mdf_R+2*I+1,Mdf_R+2*I-1)=SC(Mdf_R+2*I+1,Mdf_R+2*I-1)+
     $	C_RF*shape11*shape12

      end do 
      end do
      end do


! calculate the coupled items of slab and beam to the slab

!     对所有的浮置板循环

	dO NUF=1,NU_F

!     对每块浮置板上的单元循环,从每一个板在总的单元拓扑中 的编号开始.

      do I=(NUF-1)*NF1_E+1,NUF*NF1_E

!     每一个单元对应的节点的竖向自由度,以及其正对下方的轨道梁的竖向自由度，
!     浮置板与轨道梁有相同的单元拓扑。
 
      I_s1=Mdf_R+2*I-1
      I_s2=Mdf_R+2*I+1

	I_B1=Mdf_R+Mdf_F+2*I-1
	I_B2=Mdf_R+Mdf_F+2*I+1

!     每一个板单元上第一个支座位置编号Num_P20,上一个单元最后一个支座位置编号Num_P21,
!     最后一个支座位置编号Num_P22

      if (I.eq.((NUF-1)*NF1_E+1)) then
	Num_P20=(I-1)*(N_BS-1)+1
	else
	Num_P20=(I-1)*(N_BS-1)+1+1
	end if
	Num_P21=(I-1)*(N_BS-1)+1
	Num_P22=I*(N_BS-1)+1


!     对所有的z支座循环,就是第NUF个板上第I个单元上的支座循环

      do I_R=Num_P20,Num_P22

      shape21=((N_BS-1)-(I_R-Num_P21))/(N_BS-1)
	shape22=1-shape21

!     浮置板下第I_R个支座对浮置板的作用力，对应于浮置板的单元的两个自由度即两行对应轨道的列

      SK(I_s1,I_s1)=SK(I_s1,I_s1)+K_BF*shape21**2.0
      SK(I_s1,I_s2)=SK(I_s1,I_s2)+K_BF*shape21*shape22
      SK(I_s1,I_b1)=SK(I_s1,I_b1)-K_BF*shape21**2.0
      SK(I_s1,I_b2)=SK(I_s1,I_b2)-K_BF*shape21*shape22

      SK(I_s2,I_s1)=SK(I_s2,I_s1)+K_BF*shape21*shape22
      SK(I_s2,I_s2)=SK(I_s2,I_s2)+K_BF*shape22**2.0
      SK(I_s2,I_b1)=SK(I_s2,I_b1)-K_BF*shape21*shape22
      SK(I_s2,I_b2)=SK(I_s2,I_b2)-K_BF*shape22**2.0

      SC(I_s1,I_s1)=SC(I_s1,I_s1)+C_BF*shape21**2.0
      SC(I_s1,I_s2)=SC(I_s1,I_s2)+C_BF*shape21*shape22
      SC(I_s1,I_b1)=SC(I_s1,I_b1)-C_BF*shape21**2.0
      SC(I_s1,I_b2)=SC(I_s1,I_b2)-C_BF*shape21*shape22

      SC(I_s2,I_s1)=SC(I_s2,I_s1)+C_BF*shape21*shape22
      SC(I_s2,I_s2)=SC(I_s2,I_s2)+C_BF*shape22**2.0
      SC(I_s2,I_b1)=SC(I_s2,I_b1)-C_BF*shape21*shape22
      SC(I_s2,I_b2)=SC(I_s2,I_b2)-C_BF*shape22**2.0

!     calculate the coupled items of slab and beam to the beam

      SK(I_B1,I_s1)=SK(I_B1,I_s1)-K_BF*shape21**2.0
      SK(I_B1,I_s2)=SK(I_B1,I_s2)-K_BF*shape21*shape22
      SK(I_B1,I_b1)=SK(I_B1,I_b1)+K_BF*shape21**2.0
      SK(I_B1,I_b2)=SK(I_B1,I_b2)+K_BF*shape21*shape22

      SK(I_B2,I_s1)=SK(I_B2,I_s1)-K_BF*shape21*shape22
      SK(I_B2,I_s2)=SK(I_B2,I_s2)-K_BF*shape22**2.0
      SK(I_B2,I_b1)=SK(I_B2,I_b1)+K_BF*shape21*shape22
      SK(I_B2,I_b2)=SK(I_B2,I_b2)+K_BF*shape22**2.0

      SC(I_B1,I_s1)=SC(I_B1,I_s1)-C_BF*shape21**2.0
      SC(I_B1,I_s2)=SC(I_B1,I_s2)-C_BF*shape21*shape22
      SC(I_B1,I_b1)=SC(I_B1,I_b1)+C_BF*shape21**2.0
      SC(I_B1,I_b2)=SC(I_B1,I_b2)+C_BF*shape21*shape22

      SC(I_B2,I_s1)=SC(I_B2,I_s1)-C_BF*shape21*shape22
      SC(I_B2,I_s2)=SC(I_B2,I_s2)-C_BF*shape22**2.0
      SC(I_B2,I_b1)=SC(I_B2,I_b1)+C_BF*shape21*shape22
      SC(I_B2,I_b2)=SC(I_B2,I_b2)+C_BF*shape22**2.0

      end do
	end do
      END DO 
	return
	end 

!***********************************************************************
!                                                                      *
!  Subroutine to calculate the matrix of Rail                          *
!                                                                      *
!***********************************************************************
      subroutine Rail(LnodRS,MR,KR,CR)

      use global

      integer,intent(in)::LnodRS(Ne_R,2)

      real(8),intent(out)::MR(Mdf_R,Mdf_R),KR(Mdf_R,Mdf_R),
     $	CR(Mdf_R,Mdf_R)	

      real(8) EMR(4,4),EKR(4,4),ECR(4,4)
	
	integer Nrows(4) 

      MR=0.0
      KR=0.0
      CR=0.0 

      call RElement(EMR,EKR,ECR)
	 
      do Ie=1,Ne_R
       do K=1,2
       Nrows(K)=(LnodRS(Ie,1)-1)*2+K
       Nrows(K+2)=(LnodRS(Ie,2)-1)*2+K
       end do  
         
       do L=1,4
       I=Nrows(L)
       do K=1,4
       J=Nrows(K)
       MR(I,J)=MR(I,J)+EMR(L,K)
       KR(I,J)=KR(I,J)+EKR(L,K)
       CR(I,J)=CR(I,J)+ECR(L,K)
       end do 
       end do
 	    
      end do

      return       
      end
!***********************************************************************
!                                                                      *
! Subroutine to calculate the element matrix of Rail                   *
!                                                                      *
!***********************************************************************
      subroutine RElement(EMR,EKR,ECR) 

      use global

      real(8) EMR(4,4),EKR(4,4),ECR(4,4)

      real(8) C,D,Arfa,Beta

      EMR=0.0
      EKR=0.0
      ECR=0.0

      C=Density_R*LengthR_E/420.
      D=E_R*J_R/(LengthR_E**3)
      Arfa=40.1
      Beta=4.0e-7

!  form the EMR

      EMR(1,1)=156.0*C
      EMR(1,2)=-22.0*C*LengthR_E
      EMR(1,3)=54.0*C
      EMR(1,4)=13.0*C*LengthR_E
      EMR(2,2)=4.0*C*LengthR_E**2
      EMR(2,3)=-13.0*C*LengthR_E
      EMR(2,4)=-3.0*C*LengthR_E**2
      EMR(3,3)=156.0*C
      EMR(3,4)=22.0*C*LengthR_E
      EMR(4,4)=4.0*C*LengthR_E**2

      do I=1,4
       do J=1,I
       EMR(I,J)=EMR(J,I)
       end do
      end do

! form the EKR

      EKR(1,1)=12.0*D
      EKR(1,2)=6.0*D*LengthR_E
      EKR(1,3)=-12.0*D
      EKR(1,4)=6.0*D*LengthR_E
      EKR(2,2)=4.0*D*LengthR_E**2
      EKR(2,3)=-6.0*D*LengthR_E
      EKR(2,4)=2.0*D*LengthR_E**2
      EKR(3,3)=12.0*D
      EKR(3,4)=-6.0*D*LengthR_E
      EKR(4,4)=4.0*D*LengthR_E**2

      do I=1,4
         do J=1,I
        EKR(I,J)=EKR(J,I)
        end do
      end do

! form the ECR

       do I=1,4
       do J=1,4
      ECR(I,J)=Arfa*EMR(I,J)+Beta*EKR(I,J)
      end do
      end do

      return
      end
!***********************************************************************
!                                                                      *
! subroutine to calculate the matrix of fslab                          *
!                                                                      *
!***********************************************************************
 
      subroutine FSlab(LnodFS,MF,KF,CF)

      use global

      integer,intent(in)::LnodFS(Ne_F,2)

      real(8),intent(out)::MF(Mdf_F,Mdf_F),KF(Mdf_F,Mdf_F),
     &	CF(Mdf_F,Mdf_F)

	
      real(8) EMF(4,4),EKF(4,4),ECF(4,4)

	integer Nrows(4) 

      MF=0.0
      KF=0.0
      CF=0.0

      call FElement(EMF,EKF,ECF)
	 
      do Ie=1,Ne_F
       do K=1,2
       Nrows(K)=(LnodFS(Ie,1)-1)*2+K
       Nrows(K+2)=(LnodFS(Ie,2)-1)*2+K
       end do  
         
       do L=1,4
       I=Nrows(L)
       do K=1,4
       J=Nrows(K)
       MF(I,J)=MF(I,J)+EMF(L,K)
       KF(I,J)=KF(I,J)+EKF(L,K)
       CF(I,J)=CF(I,J)+ECF(L,K)
       end do 
       end do
 	    
      end do

      return       
      end


!***********************************************************************
!                                                                      *
! Subroutine to calculate the element matrix of fslab                  *
!                                                                      *
!***********************************************************************

      subroutine FElement(EMF,EKF,ECF) 

      use global

      real(8) EMF(4,4),EKF(4,4),ECF(4,4)

      real(8) C,D,Arfa,Beta

      EMF=0.0
      EKF=0.0
      ECF=0.0

      C=Density_F*LengthF_E/420.
      D=E_F*J_F/(LengthF_E**3)
 !     Arfa=1.169
 !     Beta=1.71e-4

      Arfa=0
      Beta=0

! form the EMB

      EMF(1,1)=156.0*C
      EMF(1,2)=-22.0*C*LengthF_E
      EMF(1,3)=54.0*C
      EMF(1,4)=13.0*C*LengthF_E
      EMF(2,2)=4.0*C*LengthF_E**2
      EMF(2,3)=-13.0*C*LengthF_E
      EMF(2,4)=-3.0*C*LengthF_E**2
      EMF(3,3)=156.0*C
      EMF(3,4)=22.0*C*LengthF_E
      EMF(4,4)=4.0*C*LengthF_E**2

      do I=1,4
        do J=1,I
        EMF(I,J)=EMF(J,I)
        end do
      end do

! form the EKB

      EKF(1,1)=12.0*D
      EKF(1,2)=6.0*D*LengthF_E
      EKF(1,3)=-12.0*D
      EKF(1,4)=6.0*D*LengthF_E
      EKF(2,2)=4.0*D*LengthF_E**2
      EKF(2,3)=-6.0*D*LengthF_E
      EKF(2,4)=2.0*D*LengthF_E**2
      EKF(3,3)=12.0*D
      EKF(3,4)=-6.0*D*LengthF_E
      EKF(4,4)=4.0*D*LengthF_E**2

       do I=1,4
        do J=1,I
        EKF(I,J)=EKF(J,I)
        end do
       end do

! form the ECB

      do I=1,4
       do J=1,4
       ECF(I,J)=Arfa*EMF(I,J)+Beta*EKF(I,J)
       end do
      end do

      return
      end



!***********************************************************************
!                                                                      *
! subroutine to calculate the matrix of Beam                           *
!                                                                      *
!***********************************************************************
      subroutine Beam(LnodBS,MB,KB,CB)

      use global

      integer,intent(in)::LnodBS(Ne_B,2)

      real(8),intent(out)::MB(Mdf_B,Mdf_B),KB(Mdf_B,Mdf_B),
     &	CB(Mdf_B,Mdf_B)

      real(8) EMB(4,4),EKB(4,4),ECB(4,4)
	
	integer Nrows(4) 

      MB=0.0
      KB=0.0
      CB=0.0 

      call BElement(EMB,EKB,ECB)
	 
      do IElement=1,Ne_B

      do k=1,2
       Nrows(k)=(LnodBs(Ielement,1)-1)*2+k
       Nrows(k+2)=(LnodBs(Ielement,2)-1)*2+k
      end do   
        
      do L=1,4
       I=Nrows(L)
       do K=1,4
       J=Nrows(K)
       MB(I,J)=MB(I,J)+EMB(L,K)
       KB(I,J)=KB(I,J)+EKB(L,K)
       CB(I,J)=CB(I,J)+ECB(L,K)
       end do
      end do

      end do

      return       
      end
!***********************************************************************
!                                                                      *
! subroutine to calculate the element matrix of beam                   *
!                                                                      *
!***********************************************************************
      subroutine BElement(EMB,EKB,ECB) 

      use global

      real(8) EMB(4,4),EKB(4,4),ECB(4,4)

      real(8) C,D,Arfa,Beta

      EMB=0.0
      EKB=0.0
      ECB=0.0

      C=Density_B*Are_B*LenB_E/420.
      D=E_B*J_B/(LenB_E**3)
 !     Arfa=1.169
 !     Beta=1.71e-4

      Arfa=0
      Beta=0

! form the EMB

      EMB(1,1)=156.0*C
      EMB(1,2)=-22.0*C*LenB_E
      EMB(1,3)=54.0*C
      EMB(1,4)=13.0*C*LenB_E
      EMB(2,2)=4.0*C*LenB_E**2
      EMB(2,3)=-13.0*C*LenB_E
      EMB(2,4)=-3.0*C*LenB_E**2
      EMB(3,3)=156.0*C
      EMB(3,4)=22.0*C*LenB_E
      EMB(4,4)=4.0*C*LenB_E**2

      do I=1,4
        do J=1,I
        EMB(I,J)=EMB(J,I)
        end do
      end do

! form the EKB

      EKB(1,1)=12.0*D
      EKB(1,2)=6.0*D*LenB_E
      EKB(1,3)=-12.0*D
      EKB(1,4)=6.0*D*LenB_E
      EKB(2,2)=4.0*D*LenB_E**2
      EKB(2,3)=-6.0*D*LenB_E
      EKB(2,4)=2.0*D*LenB_E**2
      EKB(3,3)=12.0*D
      EKB(3,4)=-6.0*D*LenB_E
      EKB(4,4)=4.0*D*LenB_E**2

       do I=1,4
        do J=1,I
        EKB(I,J)=EKB(J,I)
        end do
       end do

! form the ECB

      do I=1,4
       do J=1,4
       ECB(I,J)=Arfa*EMB(I,J)+Beta*EKB(I,J)
       end do
      end do

      return
      end
!***********************************************************************
!                                                                      *
! subroutine to calculate the matrix of vehicle                        *
!                                                                      *
!***********************************************************************
      subroutine Vehicle(MC,KC,CC)
      
      use global

      real(8),intent(out)::MC(Mdf_C,Mdf_C),KC(Mdf_C,Mdf_C),
     *	CC(Mdf_C,Mdf_C)

      real(8) EMC(10,10),EKC(10,10),ECC(10,10)

      MC=0.0
      KC=0.0
      CC=0.0

      call Vehicle_Element(EMC,ECC,EKC)

      do LC=1,Num_C
        do I=1,10
        do J=1,10
      MC(I+10*(LC-1),J+10*(LC-1))=MC(I+10*(LC-1),J+10*(LC-1))+EMC(I,J)
      KC(I+10*(LC-1),J+10*(LC-1))=KC(I+10*(LC-1),J+10*(LC-1))+EKC(I,J)
      CC(I+10*(LC-1),J+10*(LC-1))=CC(I+10*(LC-1),J+10*(LC-1))+ECC(I,J)
      end do
      end do
      end do     
   
      return
      end
!***********************************************************************
!                                                                      *
!    // Subroutine to get the EMC ECC EKC of Vechicle //               *
!                                                                      *
!***********************************************************************
      Subroutine Vehicle_Element(EMC,ECC,EKC)
      
	use global

      real*8 EMC(10,10),EKC(10,10),ECC(10,10)

      EMC=0.0
	EKC=0.0
	ECC=0.0

!    // form the EMC具体参数参见杨永斌

      EMC(1,1)=M_C
      EMC(2,2)=2.086e6
!      EMC(2,2)=5.0e6

      EMC(3,3)=M_T
      EMC(4,4)=3.93e3
!       EMC(4,4)=5.0e4

      EMC(5,5)=M_T
      EMC(6,6)=3.93e3
!       EMC(6,6)=5.0e4
      EMC(7,7)=M_W
      EMC(8,8)=M_W
      EMC(9,9)=M_W
      EMC(10,10)=M_W

!    // form the EKC

      EKC(1,1)=2.0*K_2
      EKC(1,3)=-1.0*K_2
      EKC(1,5)=-1.0*K_2
      EKC(2,2)=2.0*K_2*L_C**2
      EKC(2,3)=-1.0*K_2*L_C
      EKC(2,5)=K_2*L_C
      EKC(3,3)=K_2+2.0*K_1
      EKC(3,7)=-1.0*K_1
      EKC(3,8)=-1.0*K_1
      EKC(4,4)=2.0*K_1*L_T**2
      EKC(4,7)=-1.0*K_1*L_T
      EKC(4,8)=K_1*L_T
      EKC(5,5)=K_2+2.0*K_1
      EKC(5,9)=-1.0*K_1
      EKC(5,10)=-1.0*K_1
      EKC(6,6)=2.0*K_1*L_T**2
      EKC(6,9)=-1.0*K_1*L_T
      EKC(6,10)=K_1*L_T
      EKC(7,7)=K_1
      EKC(8,8)=K_1
      EKC(9,9)=K_1
      EKC(10,10)=K_1

	do I=1,10
	   do J=1,I
	   EKC(I,J)=EKC(J,I)
         end do
	end do

!    // form the ECC

      ECC(1,1)=2.0*C_2
      ECC(1,3)=-1.0*C_2
      ECC(1,5)=-1.0*C_2
      ECC(2,2)=2.0*C_2*L_C**2
      ECC(2,3)=-1.0*C_2*L_C
      ECC(2,5)=C_2*L_C
      ECC(3,3)=C_2+2.0*C_1
      ECC(3,7)=-1.0*C_1
      ECC(3,8)=-1.0*C_1
      ECC(4,4)=2.0*C_1*L_T**2
      ECC(4,7)=-1.0*C_1*L_T
      ECC(4,8)=C_1*L_T
      ECC(5,5)=C_2+2.0*C_1
      ECC(5,9)=-1.0*C_1
      ECC(5,10)=-1.0*C_1
      ECC(6,6)=2.0*C_1*L_T**2
      ECC(6,9)=-1.0*C_1*L_T
      ECC(6,10)=C_1*L_T
      ECC(7,7)=C_1
      ECC(8,8)=C_1
      ECC(9,9)=C_1
      ECC(10,10)=C_1

	do I=1,10
	   do J=1,I
	   ECC(I,J)=ECC(J,I)
         end do
	end do

      Return 
	End

!***********************************************************************
!                                                                      *
! Subroutine to calculate Displacement/Velocity/and accelaration       *  
! of the system at the each time                                       *
!                                                                      *
!***********************************************************************
      subroutine Steps(SM,Sk,SC,MC,KC,CC)

      use global

      use IMSL

      real(8),intent(in)::SM(Mdf_L,Mdf_L),Sk(Mdf_L,Mdf_L),
     &	SC(Mdf_L,Mdf_L)


      real(8),intent(in)::MC(Mdf_C,Mdf_C),KC(Mdf_C,Mdf_C),
     &	CC(Mdf_C,Mdf_C)

      real(8),allocatable::DL(:),VL(:),AL(:),Dc(:),Vc(:),Ac(:)
	
      real(8),allocatable::FL(:,:),Fc(:,:)
     
      real(8),allocatable::DL0(:,:),VL0(:),AL0(:),Dc0(:,:),Vc0(:),Ac0(:)

      real(8),allocatable::DL00(:),Dc00(:),DL_M(:),DL_D(:),YU(:)

	real(8),allocatable::xw(:),Dz(:),Fp(:),sp(:,:),FG(:)

	real(8),allocatable::Dis_sysDet(:),Dis_sys(:),Dis_M(:) 
	
	real(8),allocatable::F_BM(:),DL_P(:) 

      integer::N(Num_C*4),K,min_Num,max_Num

      real(8) T,p,alfa


      allocate(DL(Mdf_L),VL(Mdf_L),AL(Mdf_L),Dc(Mdf_C),Vc(Mdf_C),
     &	Ac(Mdf_C),FL(Mdf_L,1),Fc(Mdf_C,1))

      allocate(DL0(Mdf_L,1),VL0(Mdf_L),AL0(Mdf_L),Dc0(Mdf_C,1),
     &	Vc0(Mdf_C),Ac0(Mdf_C))

      allocate(DL00(Mdf_L),Dc00(Mdf_C),DL_M(Mdf_L),DL_D(Mdf_L))

      allocate(YU(5*3000))

	allocate(Sp(4*Num_C,4),Fp(Num_C*4),FG(Num_C*4),xw(Num_C*4),
     *	Dz(Num_C*4))

	allocate(Dis_sysDet(Mvar),Dis_sys(Mvar),Dis_M(Mvar))

	allocate(DL_P(4*Num_c),F_BM(4*Num_c))


      DL=0.0
      VL=0.0
      AL=0.0

      Dc=0.0
      Vc=0.0
      Ac=0.0

      DL0=0.0
      VL0=0.0
      AL0=0.0

      Dc0=0.0
      Vc0=0.0
      Ac0=0.0

	Dl00=0.0
	Dc00=0.0
	DL_M=0.0
	DL_D=0.0

	sp=0.0
	fp=0.0
	fg=0.0
	xw=0.0
	Dz=0.0

	Dis_sysDet=0.0
	Dis_sys=0.0
	Dis_M=0.0

	F_BM=0.0
	DL_P=0.0 
	N=0
      call DcInitia(Dc)

      call irregularity(YU)

! Begin the cycle of time

      Loop_time: do T=Delta_T,T_Max,Delta_T

      write(*,'(2F8.4)')T,T_Max
	
!      write(111,'(F8.4)')T
!      write(222,'(F8.4)')T
!      write(333,'(F8.4)')T
!      write(1110,'(F8.4)')T

      xw=0.0
	Do I=1,Num_C
      xw(4*(I-1)+1)=velocity*T-lw1-(I-1)*(Lc_C+lw4)
      xw(4*(I-1)+2)=xw(4*(I-1)+1)-lw2
      xw(4*(I-1)+3)=xw(4*(I-1)+1)-lw3
      xw(4*(I-1)+4)=xw(4*(I-1)+1)-lw4
      end do

      Dz=0.0
! 	write(300,'(F8.4)')T
!      write(1111,'(F8.4)')t
!     找出t时刻桥上轮对号，从第min_Num个到第max_Num，其中初始时刻车头在桥的左侧，
!    T_max时刻最后一个轮子刚出桥
      call Nm_wheel(t,xw,min_Num,max_Num)

      call Roughness(T,xw,YU,Dz)

      call Location(T,xw,min_Num,max_Num,N,Sp)

      DL_M=DL

      DL00=DL

      Dc00=Dc

      P=1.0

      K=1	
!     准备每一步中开始迭代

      Loop_iter: do while (p>=ep)

      Fp=0.0
	Fg=0.0
	Fl=0.0
	Fc=0.0
	alfa=0.0

      call Force_WR(min_Num,max_Num,N,DL00,Dc00,Dz,Sp,Fp,Fg,alfa)

      call Force_Low(min_Num,max_Num,N,Sp,Fp,FL)

      call Newmark_Low(SM,SK,SC,FL,DL,VL,AL,DL0,DL00,VL0,AL0)

      call Force_WR(min_Num,max_Num,N,DL00,Dc00,Dz,Sp,Fp,Fg,alfa)

	call Force_Wheel(min_Num,max_Num,alfa,Dc,Dz,Fg,Fc)

	call Newmark_Wheel(MC,KC,CC,Fc,Dc,Vc,Ac,Dc0,Dc00,Vc0,Ac0)

!     以下部结构为收敛准则
    
      DL_D=DL00-DL_M
      p=norm(DL_D)/norm(DL_M)
      DL_M=DL00

!     以整体位移为收敛准则

!	DO I=1,Mdf_B
!	Dis_sys(I)=DL00(I)
!	end do
!	DO I=Mdf_B+1,Mdf_B+Mdf_C
!	Dis_sys(I)=DC00(I-Mdf_B)
!	end do
      
!      Dis_sysDet=Dis_sys-Dis_M  
!      p=norm(Dis_sysDet)/norm(Dis_M)

!	DO I=1,Mdf_B
!	Dis_M(I)=DL00(I)
!	end do
!	DO I=Mdf_B+1,Mdf_B+Mdf_C
!	Dis_M(I)=DC00(I-Mdf_B)
!	end do

!     以轮桥力为收敛准则
!	DL_P=Fp-F_BM
!	p=norm(DL_P)/norm(Fp)
!	F_BM=Fp      

!      write(*,'(2F8.4,E15.6,I5)')T,T_Max,P,K
      K=K+1  

      end do loop_iter

      DL=DL00
      VL=VL0
      AL=AL0

      Dc=Dc00
      Vc=Vc0
	Ac=Ac0

!     print the response or mid-rail
      call output(T,Dc,Vc,Ac,DL,VL,AL,Fp,min_Num,max_Num,N,Sp)	

      end do loop_time

	return
	end
!***********************************************************************
!                                                                      *
! Initial displacement of vehicle                                      *
!                                                                      *
!***********************************************************************
      subroutine DcInitia(Dc)

      use global

      real(8),intent(inout)::Dc(Mdf_C)

      do I=1,Num_C
      Dc(10*(I-1)+1)=0.5*M_C*Gz/K_2
      Dc(10*(I-1)+3)=0.25*(M_C+2.0*M_T)*Gz/K_1
      Dc(10*(I-1)+5)=0.25*(M_C+2.0*M_T)*Gz/K_1

!Dc(10*(I-1)+7)=Gh*Gt**(2./3.)
!Dc(10*(I-1)+8)=Gh*Gt**(2./3.)
!Dc(10*(I-1)+9)=Gh*Gt**(2./3.)
!Dc(10*(I-1)+10)=Gh*Gt**(2./3.)

      Dc(10*(I-1)+7)=Gt/Gh
      Dc(10*(I-1)+8)=Gt/Gh
      Dc(10*(I-1)+9)=Gt/Gh
      Dc(10*(I-1)+10)=Gt/Gh
      end do
!    最终直接给赋零，以后调试程序时需注意此点。
      Dc=0.0
      return
      end
!***********************************************************************
!                                                                      *
! Subroutine ro calculate the irregularity                             *
!                                                                      *
!***********************************************************************
      subroutine irregularity(YU)

      use global

      real(8),intent(out)::YU(5*3000)
 
      real(8) YU0(5*3000)

      read(13,*)YU0

      YU=0.0

!      do I=1,5
!       do J=1,3000
!       K=(I-1)*3000+J
!       YU(K)=YU0(J)
!       end do
!      end do
      Yu=Yu0
      return
      end
!***********************************************************************
!                                                                      *
! Subroutine ro calculate the irregularity                             *
!                                                                      *
!***********************************************************************
      Subroutine Roughness(T,xw,YU,Dz)

      use global

      real(8),intent(in)::T,YU(5*3000),xw(4*num_c)

!	integer,intent(in)::min_Num,max_Num

      real(8),intent(out)::Dz(Num_C*4)

      Dz=0.0

      A0=0.005
      w0=3.1415926

      Do I=1,4*num_c
!	Dz(I)=A0/5.0*(1-cos(2.0*pi*xw(I)/1.0))
!	Dz(I)=A0/5.0*(sin(62.0*pi*xw(I)/velocity))

      KK9=int(100*abs(xw(I)))
	if(kk9.eq.0) kk9=kk9+1
      Dz(I)=YU(KK9)

      end do

 !     write(90,'(F8.4,8(E15.6))')T,Dz(1),Dz(2),Dz(3),Dz(4)
!     &	,Dz(4*num_C-3),Dz(4*num_C-2),Dz(4*num_C-1),Dz(4*num_C)

      return
      end

!******************************************************************！
!      确定桥上车轮号码
!******************************************************************！

      subroutine Nm_wheel(t,xw,min_Num,max_Num)

      use global

      real(8),intent(in)::T

      real(8),intent(in)::xw(4*Num_C)

      integer,intent(out)::min_Num,max_Num

	integer max_wheel,NN,M

      max_Num=0
      min_Num=0
	M=1
	NN=0

	max_wheel=Num_c*4
	Do 111 I=1,max_wheel
	If(xw(I).ge.length_R)goto 110
	if(xw(I).LT.0.0)goto 120
	NN=NN+1
	Goto 111
110   M=I+1
111   continue

120   min_num=M
	max_num=M+NN-1
	if (M>max_wheel)then
	M=0
      min_num=0
	max_num=0
	end if

      return
      end

!***********************************************************************
!                                                                      *
! Subroutine to calculate the location of the wheel                    *
!                                                                      *
!***********************************************************************

      subroutine Location(T,xw,min_Num,max_Num,N,Sp)

      use global

      real(8),intent(in)::T,xw(4*Num_C)

      integer,intent(in)::min_Num,max_Num

      integer,intent(out)::N(Num_C*4)

      real(8),intent(out)::Sp(4*Num_C,4)

      real(8) x(4*Num_C),R1(4*Num_C)

	sp=0.0
	x=0.0
	r1=0.0
      N=0
      do I=min_Num,max_Num


!     第I轮对所在位置的轨道单元号

!      if(xw(I).eq.length_B)then
!	N(I)=NE_B
!	else

      R1(I)=xw(I)/LengthR_E
         if(Mod(R1(I),1.0).gt.0.9999)then
         R1(I)=R1(I)+0.0001
         end if
         N(I)=int(R1(I))

         if((N(I)*1.0).LT.R1(I)) then
         N(I)=N(I)+1
         end if
!	end if


!      write(1111,'(1(I4),3x,2(F15.6),I4)')I,xw(I),R1(I),N(I)
!     第I轮对所在位置局部坐标

      X(I)=(LengthR_E-(N(I)*LengthR_E-Xw(I)))/LengthR_E

!     第I轮对所在位置桥梁插值函数

      Sp(I,1)=1-3.0*X(I)**2+2.0*x(I)**3
      Sp(I,2)=LengthR_E*(x(I)-2.0*x(I)**2+x(I)**3)
      Sp(I,3)=3.0*x(I)**2-2.0*x(I)**3
      Sp(I,4)=LengthR_E*(x(I)**3-x(I)**2)
	end do
      return
      end
!***********************************************************************
!                                                                      *
! Subroutine to get the force between wheel and beam                   *
!                                                                      *
!***********************************************************************
      subroutine Force_WR(min_Num,max_Num,N,DL,Dc,Dz,Sp,Fp,Fg,alfa)

      use global

      real(8),intent(in)::DL(Mdf_L),Dc(Mdf_C),Dz(Num_C*4),Sp(4*Num_C,4)

      integer,intent(in)::N(Num_C*4),min_Num,max_Num

      real(8),intent(out)::Fp(Num_C*4),Fg(Num_C*4),alfa

      real(8) Zr(Num_C*4),zh(Num_C*4),KK,fh(Num_C*4)

	INTEGER kk1,kk2

      Fp=0.0
	Fg=0.0
      Zr=0.0
      zh=0.0
	alfa=0.0
	fh=0.0

      do I=min_Num,max_Num

	    ! 轮对I对应的广义位移向量的位置确定
   
      kk=(I-0.25)/4.0

	kk1=int(kk)+1

      kk2=kk1*6+I

      M=N(I)

      Zr(I)=Sp(I,1)*DL(2*M-1)+Sp(I,2)*DL(2*M)+Sp(I,3)*DL(2*M+1)+
     &	Sp(I,4)*DL(2*M+2)

      zh(I)=Dc(kk2)-Zr(I)+Dz(I)


      Fh(I)=zh(I)*Gh
!     Fp(I)=(zh(I)/Gh)**1.5

      if ((Fh(I)+Gt)>=0.0) then

	Fg(I)=Fh(I)
      Fp(I)=Fh(I)+Gt
	alfa=0.0
 
      else

      Fp(I)=0.0
	Fg(I)=0.0
      alfa=1.0
      end if
!      write(1110,'(2(I4),3x,7(E15.6))')I,kk2,
!     *	DC(kk2),zh(I),FH(I),Gt,Fg(I),Fp(I),alfa

      end do

      return
      end
!***********************************************************************
!                                                                      *
! Subroutine to get the force of beam                                  *
!                                                                      *
!***********************************************************************
      subroutine Force_Low(min_Num,max_Num,N,Sp,Fp,FL)

      use global

      real(8),intent(in)::Sp(4*Num_C,4),Fp(Num_C*4)

      integer,intent(in)::N(Num_C*4),min_Num,max_Num

      real(8),intent(out)::FL(Mdf_L,1)

      FL=0.0

      do I=min_Num,max_Num

      M=N(I)

      FL(2*M-1,1)=FL(2*M-1,1)+Fp(I)*Sp(I,1)
      FL(2*M,1)=FL(2*M,1)+Fp(I)*Sp(I,2)
      FL(2*M+1,1)=FL(2*M+1,1)+Fp(I)*Sp(I,3)
      FL(2*M+2,1)=FL(2*M+2,1)+Fp(I)*Sp(I,4)

      end do

      return
      end

!***********************************************************************
!                                                                      *
! Subroutine of newmark-b methods                                      *
!                                                                      *
!***********************************************************************
      subroutine Newmark_Low(SM,SK,SC,FL,DL,VL,AL,DL0,DL00,VL0,AL0)

      use IMSL

      use global

      real(8),intent(in)::SM(Mdf_L,Mdf_L),SK(Mdf_L,Mdf_L),
     &	SC(Mdf_L,Mdf_L),FL(Mdf_L,1)
	
      real(8),intent(in)::DL(Mdf_L),VL(Mdf_L),AL(Mdf_L)

      real(8),intent(out)::DL0(Mdf_L,1),DL00(Mdf_L),VL0(Mdf_L),
     &	AL0(Mdf_L)

      real(8),allocatable::SSK(:,:),SForce(:,:),Sdis0(:),Sdis1(:)

      real(8) Del,Alf,A0,A1,A2,A3,A4,A5,A6,A7

	integer Mdf_B0

      allocate(SSK(Mdf_L,Mdf_L),SForce(Mdf_L,1),Sdis0(Mdf_L),
     &	Sdis1(Mdf_L))

   
      SForce=0.0
      SSK=0.0
      Sdis0=0.0
      Sdis1=0.0

      DL0=0.0
      DL00=0.0
      VL0=0.0
      AL0=0.0

!     计算参数 

       Del=0.505
       Alf=0.25250625 

      A0=1.0/(Alf*Delta_T**2.0)
      A1=Del/(Alf*Delta_T)
      A2=1.0/(Alf*Delta_T)
      A3=1.0/(2.0*Alf)-1.0
      A4=Del/Alf-1.0
      A5=Delta_T/2.0*(Del/Alf-2.0)
      A6=Delta_T*(1.0-Del)
      A7=Del*Delta_T

!     计算有效刚度阵

      do I=1,Mdf_L
       do J=1,Mdf_L
       SSK(I,J)=SK(I,J)+A0*SM(I,J)+A1*SC(I,J)
       end do
      end do

! 计算有效位移向量

      do I=1,Mdf_L
       Sdis0(I)=A0*DL(I)+A2*VL(I)+A3*AL(I)
      end do

! 计算有效速度向量

       do I=1,Mdf_L
       Sdis1(I)=A1*DL(I)+A4*VL(I)+A5*AL(I)
       end do

! 计算有效力阵

      do I=1,Mdf_L
        do L=1,Mdf_L
       SForce(I,1)=SForce(I,1)+SM(I,L)*Sdis0(L)+SC(I,L)*Sdis1(L)
       end do
       SForce(I,1)=SForce(I,1)+FL(I,1)
      end do

! 定义轨道梁的边界条件(多跨简支)

!      do I=1,Mdf_L
!      if(I.NE.Mdf_R+mdf_F+1) then
!      SSK(Mdf_R+mdf_F+1,I)=0.0
!      SSK(I,Mdf_R+mdf_F+1)=0.0
!      end if
!      end do

!      SSK(Mdf_R+mdf_f+1,Mdf_R+mdf_f+1)=1.0
!      SForce(Mdf_R+mdf_f+1,1)=0.0

!     Fixed bridge
      do I= Mdf_R+mdf_F+1,Mdf_L
	   do J = Mdf_R+mdf_F+1,Mdf_L
	         if(I.NE.J) then
                 SSK(I,J)=0.0
                 SForce(I,1)=0.0
	         else if (I.EQ.J) then
                 SSK(I,J)=1.0
                 SForce(I,1)=0.0
			 end if
        end do
	end do

!      do J=1,Num_B
!       Mdf_B0=Mdf_R+mdf_f+2*(int((J*Length_B/Num_B)/LenB_E)+1)
!       do I=1,Mdf_L
!       if(I.NE.Mdf_B0-1) then
!       SSK(Mdf_B0-1,I)=0.0
!       SSK(I,Mdf_B0-1)=0.0
!       end if
!       SSK(Mdf_B0-1,Mdf_B0-1)=1.0
!       SForce(Mdf_B0-1,1)=0.0  
!      end do
!      end do

!     调用子程序解位移 

      call LIN_SOL_GEN(SSK,SForce,DL0)

!     计算速度与加速度向量
        
      do J=1,Mdf_L
      AL0(J)=A0*(DL0(J,1)-DL(J))-A2*VL(J)-A3*AL(J)
      VL0(J)=VL(J)+A6*AL(J)+A7*AL0(J)
      end do

      do I=1,Mdf_L
      DL00(I)=DL0(I,1)
      end do

      return
      end
!***********************************************************************
!                                                                      *
! Subroutine to get the force of wheel                                 *
!                                                                      *
!***********************************************************************
      subroutine Force_Wheel(min_Num,max_Num,alfa,Dc,Dz,Fg,Fc)

      use global

      real(8),intent(in)::Fg(Num_C*4),alfa,Dc(Mdf_C),Dz(Num_C*4)

	integer,intent(in)::min_Num,max_Num

      real(8),intent(out)::Fc(Mdf_C,1)

	real(8) kk,Fcg,pp,pp1,ll,dff,dff2

	integer kk1,kk2,N_min,kk3,kk4,kk5,kk6,I1,I2

      Fc=0.0

!      do I=1,Num_C

!      Fc(10*(I-1)+1,1)=M_C*Gz
!      Fc(10*(I-1)+3,1)=M_T*Gz
!      Fc(10*(I-1)+5,1)=M_T*Gz

!      do J=1,4

!      Fc(10*(I-1)+6+J,1)=M_W*Gz
!      end do
!	end do

!     桥上轮子循环，确定期受力情况
	loop_wheel1:Do I=min_Num,max_Num

!     轮对I对应的广义位移向量的位置确定
   
      kk=(I-0.25)/4.0

	kk1=int(kk)+1

      kk2=kk1*6+I

	Fc(KK2,1)=Fc(KK2,1)-Fg(I)+alfa*M_W*Gz

!	write(111,'(2(I4),3(e15.6))')I,kk2,alfa,Fg(I),Fc(kk2,1)
!	Fc(KK2,1)=Fc(KK2,1)-Fg(I)
	end do loop_wheel1

!    对出桥的轮子循环
!	N_min=min_Num-1
    
!      If(N_min.Ge.1) then 

!	Loop_wheel2: Do I=1,N_min

!     判断出桥后第i个轮子是否脱离

   ! 轮对I对应的广义位移向量的位置确定
   
!      LL=(I-0.25)/4.0

!	I1=int(LL)+1

!      I2=I1*6+I

!      Dff=Gh*(Dc(I2)+Dz(I))+Gt

!      if (Dff>=0.0) then
!      Fc(I2,1)=Fc(I2,1)-Gh*(Dc(I2)+Dz(I))
!      else
!      Fc(I2,1)=Fc(I2,1)+M_w*Gz

!      end if
!      write(222,'(2(I4),2(e15.6))')I,I2,Fc(I2,1),M_w*Gz
!      end do loop_wheel2

!      end if

!    对未进桥轮子循环
    
!      If(max_Num.Lt.(4*num_C)) then 

!	Loop_wheel3: Do I=max_Num+1,4*num_C

!     判断未进桥第i个轮子是否脱离

!     轮对I对应的广义位移向量的位置确定
   
!      pp=(I-0.25)/4.0
!	kk3=int(pp)+1

 !     kk4=kk3*6+I

!      Dff2=Gh*(Dc(kk4)+Dz(I))+Gt

!      if (Dff2>=0.0) then
!      Fc(kk4,1)=Fc(kk4,1)-Gh*(Dc(kk4)+Dz(I))
!      else
!      Fc(kk4,1)=Fc(kk4,1)+M_w*Gz

!      end if

!      write(333,'(2(I4),2(e15.6))')I,kk4,Fc(kk4,1),M_w*Gz
!      end do loop_wheel3
!      end if

      return
      end
!***********************************************************************
!                                                                      *
! Subroutine of newmark-b methods                                      *
!                                                                      *
!***********************************************************************
      subroutine Newmark_Wheel(MC,KC,CC,Fc,Dc,Vc,Ac,Dc0,Dc00,Vc0,Ac0)

      use IMSL

      use global

      real(8),intent(in)::MC(Mdf_C,Mdf_C),KC(Mdf_C,Mdf_C),
     &	CC(Mdf_C,Mdf_C),Fc(Mdf_C,1)
	
      real(8),intent(in)::Dc(Mdf_C),Vc(Mdf_C),Ac(Mdf_C)

      real(8),intent(out)::Dc0(Mdf_C,1),Dc00(Mdf_C),Vc0(Mdf_C),
     &	Ac0(Mdf_C)

      real(8),allocatable::SSK(:,:),SForce(:,:),Sdis0(:),Sdis1(:)

      real(8) Del,Alf,A0,A1,A2,A3,A4,A5,A6,A7

      allocate(SSK(Mdf_C,Mdf_C),SForce(Mdf_C,1),Sdis0(Mdf_C),
     &	Sdis1(Mdf_C))
   
      SForce=0.0
      SSK=0.0
      Sdis0=0.0
      Sdis1=0.0

      Dc0=0.0
      Dc00=0.0
      Vc0=0.0
      Ac0=0.0

!     计算参数 

      Del=0.505
      Alf=0.25250625 

      A0=1.0/(Alf*Delta_T**2.0)
      A1=Del/(Alf*Delta_T)
      A2=1.0/(Alf*Delta_T)
      A3=1.0/(2.0*Alf)-1.0
      A4=Del/Alf-1.0
      A5=Delta_T/2.0*(Del/Alf-2.0)
      A6=Delta_T*(1.0-Del)
      A7=Del*Delta_T

!     计算有效刚度阵

      do I=1,Mdf_C
       do J=1,Mdf_C
       SSK(I,J)=KC(I,J)+A0*MC(I,J)+A1*CC(I,J)
       end do
      end do

!     计算有效位移向量

      do I=1,Mdf_C
       Sdis0(I)=A0*Dc(I)+A2*Vc(I)+A3*Ac(I)
      end do

!     计算有效速度向量

      do I=1,Mdf_C
       Sdis1(I)=A1*Dc(I)+A4*Vc(I)+A5*Ac(I)
      end do

!     计算有效力阵


      do I=1,Mdf_C
        do L=1,Mdf_C
       SForce(I,1)=SForce(I,1)+MC(I,L)*Sdis0(L)+CC(I,L)*Sdis1(L)
       end do
       SForce(I,1)=SForce(I,1)+Fc(I,1)
      end do

!     调用子程序解位移 

      call LIN_SOL_GEN(SSK,SForce,Dc0)

!     计算速度与加速度向量
        
      do J=1,Mdf_C
      Ac0(J)=A0*(Dc0(J,1)-Dc(J))-A2*Vc(J)-A3*Ac(J)
      Vc0(J)=Vc(J)+A6*Ac(J)+A7*Ac0(J)
      end do

      do I=1,Mdf_C
      Dc00(I)=Dc0(I,1)
      end do

      return
      end
!***********************************************************************
!                                                                      *
! Subroutine to print the result                                       *
!                                                                      *
!***********************************************************************
      subroutine output(T,Dc,Vc,Ac,DL,VL,AL,Fp,min_Num,max_Num,N,Sp)

      use global

      real(8),intent(in)::T,Dc(Mdf_C),Vc(Mdf_C),Ac(Mdf_C),D
     &	L(Mdf_L),VL(Mdf_L),AL(Mdf_L),Fp(Num_C*4),Sp(4*Num_C,4)

	integer,intent(in)::N(Num_C*4),min_Num,max_Num

      integer iostat_value,node1,node2
	real(8) F1, F2,F3,F4
	character( len = 2 ) :: cTemp1,cTemp2 
	Logical opened1,opened2

	F1=0.0
	F2=0.0
	F3=0.0
	F4=0.0

 !     print the response of vehicle body of the first and the last vehicle
      write(30,'(F8.4,3x,3(E15.6))')T,
     *	DC(1),VC(1),AC(1)
!     *    ,DC(Mdf_C-9),VC(Mdf_C-9),AC(Mdf_C-9)

!      print the response of turn of the first and the last vehicle
      write(40,'(F8.4,3x,6(E15.6))')T,
     *	DC(3),VC(3),AC(3),DC(5),VC(5),AC(5)
!     *	,DC(Mdf_C-7),VC(Mdf_C-7),AC(Mdf_C-7)
!     *	,DC(Mdf_C-5),VC(Mdf_C-5),AC(Mdf_C-5)

!     print the response of wheels of the first and the last vehicle
      write(50,'(F8.4,3x,12(E15.6))')T,
     * 	DC(7),VC(7),AC(7),DC(8),VC(8),AC(8),
     *	DC(9),VC(9),AC(9),DC(10),VC(10),AC(10)
!     * 	,DC(Mdf_C-3),VC(Mdf_C-3),AC(Mdf_C-3),
!     *	DC(Mdf_C-2),VC(Mdf_C-2),AC(Mdf_C-2),
!     *	DC(Mdf_C-1),VC(Mdf_C-1),AC(Mdf_C-1),
!     *	DC(Mdf_C),VC(Mdf_C),AC(Mdf_C)

! print the force between rail and wheel
      write(60,'(F8.4,4(e15.6))')T,Fp(1),Fp(2),Fp(3),Fp(4)
!     &	,Fp(4*num_C-3),Fp(4*num_C-2),Fp(4*num_C-1),Fp(4*num_C) 

      do I=min_Num,max_Num
         M=N(I)
         node1 = M
         node2 = M+1
         F1=Fp(I)*Sp(I,1)
         F2=Fp(I)*Sp(I,2)
         F3=Fp(I)*Sp(I,3)
         F3=Fp(I)*Sp(I,4)

!     open file for write force on each nodes
!     found each file according to the node1 and node2 Name1 = "b1.dat", Name2 = "b2.dat"
      write( cTemp1,'(i2)' ) node1 
	inquire(file = 'b' // trim(adjustl( cTemp1 )) //'.txt',
     &	Opened = opened1)
	if(.NOT. opened1) then
         open (100*node1, file = 'b' // trim(adjustl( cTemp1 )) //'.txt'
     &	   ,status = 'unknown' ) 
      end if

	write (100*node1,99,IOSTAT=iostat_value)T,node1,F1,F2

      write( cTemp2,'(i2)' ) node2 
	inquire(file = 'b' // trim(adjustl( cTemp2)) //'.txt',
     &	Opened = opened2)
!       if file is not opened, then open it        
	if(.NOT. opened2) then
          open ( 100*node2, file = 'b'//trim(adjustl(cTemp2))//'.txt',
     &		status='unknown') 
      end if
    
      write (100*node2,99,IOSTAT = iostat_value)T,node2,F3,F4

99    FORMAT(2X,1pe15.6,I10,3X,1pe15.6,3X,1pe15.6) 
      End DO



	return
      end




