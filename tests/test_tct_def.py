from enum import IntEnum
from pitct.tct_def import TCTProgram


class TestTCTProgramValues:
    def test_create(self):
        assert TCTProgram.CREATE == 0

    def test_selfloop(self):
        assert TCTProgram.SELFLOOP == 1

    def test_trim(self):
        assert TCTProgram.TRIM == 2

    def test_print_des(self):
        assert TCTProgram.PRINT_DES == 3

    def test_sync(self):
        assert TCTProgram.SYNC == 4

    def test_meet(self):
        assert TCTProgram.MEET == 5

    def test_supcon(self):
        assert TCTProgram.SUPCON == 6

    def test_all_events(self):
        assert TCTProgram.ALL_EVENTS == 7

    def test_mutex(self):
        assert TCTProgram.MUTEX == 8

    def test_complement(self):
        assert TCTProgram.COMPLEMENT == 9

    def test_non_conflict(self):
        assert TCTProgram.NON_CONFLICT == 10

    def test_condat(self):
        assert TCTProgram.CONDAT == 11

    def test_supreduce(self):
        assert TCTProgram.SUPREDUCE == 12

    def test_isomorph(self):
        assert TCTProgram.ISOMORPH == 13

    def test_print_dat(self):
        assert TCTProgram.PRINT_DAT == 14

    def test_getdes_param(self):
        assert TCTProgram.GETDES_PARAM == 15

    def test_supconrobs(self):
        assert TCTProgram.SUPCONROBS == 16

    def test_project(self):
        assert TCTProgram.PROJECT == 17

    def test_localize(self):
        assert TCTProgram.LOCALIZE == 18

    def test_minstate(self):
        assert TCTProgram.MINSTATE == 19

    def test_force(self):
        assert TCTProgram.FORCE == 20

    def test_convert(self):
        assert TCTProgram.CONVERT == 21

    def test_supnorm(self):
        assert TCTProgram.SUPNORM == 22

    def test_supscop(self):
        assert TCTProgram.SUPSCOP == 23

    def test_can_qc(self):
        assert TCTProgram.CAN_QC == 24

    def test_obs(self):
        assert TCTProgram.OBS == 25

    def test_nat_obs(self):
        assert TCTProgram.NAT_OBS == 26

    def test_sup_obs(self):
        assert TCTProgram.SUP_OBS == 27

    def test_bfs_recode(self):
        assert TCTProgram.BFS_RECODE == 28

    def test_ext_suprobs(self):
        assert TCTProgram.EXT_SUPROBS == 29


class TestTCTProgramType:
    def test_is_int_enum_subclass(self):
        assert issubclass(TCTProgram, IntEnum)

    def test_members_are_int(self):
        for member in TCTProgram:
            assert isinstance(member, int)

    def test_total_count(self):
        assert len(TCTProgram) == 30

    def test_can_use_as_int(self):
        programs = [0] * 32
        programs[TCTProgram.SUPCON] = "supcon_func"
        assert programs[6] == "supcon_func"
