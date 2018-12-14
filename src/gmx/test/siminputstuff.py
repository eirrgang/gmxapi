#!/usr/bin/env python

import os
import tempfile
import gmx.fileio
import gmx.core
from gmx.data import tpr_filename

if __name__ == "__main__":
    md = gmx.workflow.from_tpr(tpr_filename)
    print(md.workspec)
    #
    # tprfile = gmx.fileio.TprFile(tpr_filename, 'r')
    # with tprfile as fh:
    #     originalParams = fh._tprFileHandle.params()
    # print(originalParams.extract()["init-step"])
    #
    # # Note GROMACS silently errors out if provided file name does not end in ".tpr"
    # fd, tempfilepath = tempfile.mkstemp(suffix=".tpr")
    # os.close(fd)
    #
    # originalParams.set('init-step', 1)
    #
    # gmx.core.write_tprfile(tempfilepath, originalParams)
    #
    # tprfile = gmx.fileio.TprFile(tempfilepath, 'r')
    # with tprfile as fh:
    #     newParams = fh._tprFileHandle.params()
    #     print(originalParams.extract()["init-step"])
    #
    # os.unlink(tempfilepath)

