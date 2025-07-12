from enum import IntEnum


class TCTProgram(IntEnum):
    CREATE          = 0
    SELFLOOP        = 1
    TRIM            = 2
    PRINT_DES       = 3
    SYNC            = 4
    MEET            = 5
    SUPCON          = 6
    ALL_EVENTS      = 7
    MUTEX           = 8
    COMPLEMENT      = 9
    NON_CONFLICT    = 10
    CONDAT          = 11
    SUPREDUCE       = 12
    ISOMORPH        = 13
    PRINT_DAT       = 14
    GETDES_PARAM    = 15
    SUPCONROBS      = 16
    PROJECT         = 17
    LOCALIZE        = 18
    MINSTATE        = 19
    FORCE           = 20
    CONVERT         = 21
    SUPNORM         = 22
    SUPSCOP         = 23
    CAN_QC          = 24
    OBS             = 25
    NAT_OBS         = 26
    SUP_OBS         = 27
    BFS_RECODE      = 28
    EXT_SUPROBS     = 29
    # create_program, selfloop_program, trim_program,   printdes_program,
    # sync_program,   meet_program,     supcon_program, allevents_program,
    # mutex_program, complement_program, nonconflict_program, condat_program,
    # supreduce_program, isomorph_program, printdat_program, getdes_parameter_program,
    # supconrobs_program, project_program, localize_program, minstate_program,
    # force_program, convert_program, supnorm_program, supscop_program, 
    # canQC_program, obs_program, natobs_program, supobs_program, bfs_recode_program,
    # ext_suprobs_program