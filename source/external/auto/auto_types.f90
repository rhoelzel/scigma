MODULE AUTO_TYPES

  IMPLICIT NONE

  INTEGER, PARAMETER::NPARX=36
  
  TYPE INDEXVAR
     CHARACTER(13) INDEX
     DOUBLE PRECISION VAR
  END TYPE INDEXVAR

  TYPE INDEXMVAR
     CHARACTER(13) INDEX
     DOUBLE PRECISION, POINTER :: VAR(:)
  END TYPE INDEXMVAR

  TYPE INDEXSTR
     INTEGER INDEX
     CHARACTER(13) STR
  END TYPE INDEXSTR

  TYPE SOLUTION
     INTEGER :: IBR, NTOT, ITP, LAB, NFPR, ISW, NTPL, NAR, NROWPR, NTST, NCOL,&
          NPAR, NPARI, NDM, IPS, IPRIV
     DOUBLE PRECISION, DIMENSION(:,:), POINTER :: UPS, UDOTPS
     DOUBLE PRECISION, DIMENSION(:), POINTER :: TM
     DOUBLE PRECISION, DIMENSION(:), POINTER :: RLDOT, PAR
     INTEGER, DIMENSION(:), POINTER :: ICP
     TYPE(SOLUTION), POINTER :: NEXT
  END TYPE SOLUTION

  TYPE HCONST_TYPE
     INTEGER NUNSTAB,NSTAB,IEQUIB,ITWIST,ISTART
     INTEGER, POINTER :: IREV(:),IFIXED(:),IPSI(:)
  END TYPE HCONST_TYPE

  TYPE BVP_TYPE
     INTEGER, ALLOCATABLE :: NRTN(:)
     INTEGER IRTN
  END TYPE BVP_TYPE
  
  TYPE IO_TYPE
     TYPE(SOLUTION), POINTER :: ROOTSOL, CURSOL
     INTEGER ::MBR=0, MLAB=0
  END TYPE IO_TYPE

  TYPE SUPPORT_TYPE
     DOUBLE PRECISION, ALLOCATABLE :: DTV(:),P0V(:,:),P1V(:,:)
     COMPLEX(KIND(1.0D0)), ALLOCATABLE :: EVV(:)
  END TYPE SUPPORT_TYPE

  TYPE HOMCONT_TYPE
     INTEGER:: ITWIST=0,ISTART=5,IEQUIB=1,NFIXED=0,&
          NPSI=0,NUNSTAB=-1,NSTAB=-1,NREV=0
     INTEGER, ALLOCATABLE :: IREV(:),IFIXED(:),IPSI(:)
     LOGICAL :: INITCNST=.FALSE.
     DOUBLE PRECISION :: COMPZERO
  END TYPE HOMCONT_TYPE
  
  TYPE AUTOPARAMETERS
     SEQUENCE

     DOUBLE PRECISION DS, DSMIN, DSMAX, RDS, RL0, RL1, A0, A1
     DOUBLE PRECISION EPSL, EPSU, EPSS

     DOUBLE PRECISION DET, FLDF, HBFF, BIFF, SPBF

     INTEGER NDIM, IPS, IRS, ILP, NTST, NCOL, IAD, IADS, ISP, ISW
     INTEGER IPLT, NBC, NINT, NMX, NUZR, NPR, MXBF, IIS, IID, ITMX, ITNW
     INTEGER NWTN, JAC, NPAR, NREV

     INTEGER NDM, NPARI, ITP, ITPST, NFPR, IBR, NTOT
     INTEGER NINS, LAB, NICP, NTEST

  END TYPE AUTOPARAMETERS

  TYPE AUTOCONTEXT
     ! these are the AUTO constants, *exactly* as given in the fort.2 or c. file
     INTEGER NDIM,IPS,IRS,ILP
     INTEGER NTST,NCOL,IAD,ISP,ISW,IPLT,NBC,NINT
     INTEGER NMX
     DOUBLE PRECISION RL0,RL1,A0,A1
     INTEGER NPR,MXBF,IIS,IID,ITMX,ITNW,NWTN,JAC
     DOUBLE PRECISION EPSL,EPSU,EPSS
     DOUBLE PRECISION DS,DSMIN,DSMAX
     INTEGER IADS,NPAR,IBR,LAB
     LOGICAL::NEWCFILE=.FALSE.

     CHARACTER(13), ALLOCATABLE :: ICU(:)
     TYPE(INDEXVAR),ALLOCATABLE :: IVTHL(:),IVTHU(:),UVALS(:),PARVALS(:)
     TYPE(INDEXMVAR),ALLOCATABLE :: IVUZR(:)
     TYPE(INDEXMVAR),ALLOCATABLE :: IVUZSTOP(:)

     TYPE(INDEXSTR), ALLOCATABLE :: unames(:), parnames(:)
     CHARACTER(13), ALLOCATABLE :: SP(:), STOPS(:)
     CHARACTER(13) :: SIRS, TY
     CHARACTER(256) :: EFILE, SFILE, SVFILE, DATFILE

     INTEGER::NIAP=36
     INTEGER::NRAP=16

     ! these are the AUTO parameters, as used to be in the IAP and RAP arrays.
     ! NOTE: for MPI:
     ! * SEQUENCE is necessary, first comes double precision, then integer
     ! * DS should be the first double precision variable
     ! * NDIM should be the first integer variable  
     ! * NIAP and NRAP are used!
     ! * For alignment, always keep an even number of integer variables.
     !   (NIAP should be even;  make the structure length a multiple of its
     !    largest element)
     TYPE(AUTOPARAMETERS) AP

     ! HomCont constants:
     TYPE(HCONST_TYPE)HCONST

     ! Variables of the BVP module
     TYPE(BVP_TYPE) BVP
     
     ! Variables of the IO module
     TYPE(IO_TYPE) IO

     ! Variables of the SUPPORT module
     TYPE(SUPPORT_TYPE) SUPPORT

     ! Variables of the HOMCONT module
     TYPE(HOMCONT_TYPE) HOMCONT

     ! ID of the AUTOCONTEXT
     INTEGER ID
     ! IO units for the different files
     INTEGER CUNIT, SUNIT, BUNIT, DUNIT

  END TYPE AUTOCONTEXT

END MODULE AUTO_TYPES
