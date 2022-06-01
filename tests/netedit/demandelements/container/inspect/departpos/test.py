#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2009-2022 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    test.py
# @author  Pablo Alvarez Lopez
# @date    2019-07-16

# import common functions for netedit tests
import os
import sys

testRoot = os.path.join(os.environ.get('SUMO_HOME', '.'), 'tests')
neteditTestRoot = os.path.join(
    os.environ.get('TEXTTEST_HOME', testRoot), 'netedit')
sys.path.append(neteditTestRoot)
import neteditTestFunctions as netedit  # noqa

# Open netedit
neteditProcess, referencePosition = netedit.setupAndStart(neteditTestRoot, ['--gui-testing-debug-gl'])

# force save additionals
netedit.forceSaveAdditionals()

# go to demand mode
netedit.supermodeDemand()

# go to container mode
netedit.containerMode()

# create container using three edges
netedit.leftClick(referencePosition, 274, 400)
netedit.leftClick(referencePosition, 180, 60)

# press enter to create container
netedit.typeEnter()

# change zoom
netedit.setZoom("0", "-6", "5")

# go to inspect mode
netedit.inspectMode()

# inspect container
netedit.leftClick(referencePosition, 310, 140)

# change departLane with an invalid value
netedit.modifyAttribute(netedit.attrs.container.inspect.departPos, "", False)

# change departLane with an invalid value
netedit.modifyAttribute(netedit.attrs.container.inspect.departPos, "dummyPos", False)

# change departLane with an invalid value
netedit.modifyAttribute(netedit.attrs.container.inspect.departPos, "500", False)

# change departLane with a valid value
netedit.modifyAttribute(netedit.attrs.container.inspect.departPos, "20", False)

# Check undo
netedit.undo(referencePosition, 3)
netedit.redo(referencePosition, 3)

# save network
netedit.saveNetwork(referencePosition)

# save containers
netedit.saveRoutes(referencePosition)

# save additionals
netedit.saveAdditionals(referencePosition)

# quit netedit
netedit.quit(neteditProcess)
