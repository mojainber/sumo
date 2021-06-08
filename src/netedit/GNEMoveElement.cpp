/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2021 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    GNEMoveElement.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Mar 2020
///
// Class used for move shape elements
/****************************************************************************/
#include <netedit/elements/network/GNEEdge.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/GNEViewNet.h>

#include "GNEMoveElement.h"


// ===========================================================================
// GNEMoveOperation method definitions
// ===========================================================================

GNEMoveOperation::GNEMoveOperation(GNEMoveElement* _moveElement,
                                   const Position _originalPosition) :
    moveElement(_moveElement),
    originalShape({_originalPosition}),
    firstLane(nullptr),
    originalStarPos(INVALID_DOUBLE),
    secondLane(nullptr),
    originalEndPos(INVALID_DOUBLE),
    shapeToMove({_originalPosition}),
    allowChangeLane(false) {
}


GNEMoveOperation::GNEMoveOperation(GNEMoveElement* _moveElement,
                                   const PositionVector _originalShape) :
    moveElement(_moveElement),
    originalShape(_originalShape),
    shapeToMove(_originalShape),
    firstLane(nullptr),
    originalStarPos(INVALID_DOUBLE),
    secondLane(nullptr),
    originalEndPos(INVALID_DOUBLE),
    allowChangeLane(false) {
}


GNEMoveOperation::GNEMoveOperation(GNEMoveElement* _moveElement,
                                   const PositionVector _originalShape,
                                   const std::vector<int> _originalgeometryPoints,
                                   const PositionVector _shapeToMove,
                                   const std::vector<int> _geometryPointsToMove) :
    moveElement(_moveElement),
    originalShape(_originalShape),
    originalGeometryPoints(_originalgeometryPoints),
    firstLane(nullptr),
    originalStarPos(INVALID_DOUBLE),
    secondLane(nullptr),
    originalEndPos(INVALID_DOUBLE),
    shapeToMove(_shapeToMove),
    geometryPointsToMove(_geometryPointsToMove),
    allowChangeLane(false) {
}


GNEMoveOperation::GNEMoveOperation(GNEMoveElement* _moveElement,
                                   const GNELane* _lane,
                                   const double _originalStarPos,
                                   const bool _allowChangeLane) :
    moveElement(_moveElement),
    firstLane(_lane),
    originalStarPos(_originalStarPos * _lane->getLengthGeometryFactor()),
    secondLane(nullptr),
    originalEndPos(INVALID_DOUBLE),
    allowChangeLane(_allowChangeLane) {
}


GNEMoveOperation::GNEMoveOperation(GNEMoveElement* _moveElement,
                                   const GNELane* _lane,
                                   const double _originalStarPos,
                                   const double _originalEndPos,
                                   const bool _allowChangeLane) :
    moveElement(_moveElement),
    firstLane(_lane),
    originalStarPos(_originalStarPos * _lane->getLengthGeometryFactor()),
    secondLane(nullptr),
    originalEndPos(_originalEndPos * _lane->getLengthGeometryFactor()),
    allowChangeLane(_allowChangeLane) {
}


GNEMoveOperation::GNEMoveOperation(GNEMoveElement* _moveElement,
                                   const GNELane* _firstLane,
                                   const double _firstStartPos,
                                   const GNELane* _secondLane,
                                   const double _secondStartPos) :
    moveElement(_moveElement),
    firstLane(_firstLane),
    originalStarPos(_firstStartPos * _firstLane->getLengthGeometryFactor()),
    secondLane(secondLane),
    originalEndPos(_secondStartPos * _secondLane->getLengthGeometryFactor()),
    allowChangeLane(false) {
}


GNEMoveOperation::~GNEMoveOperation() {}

// ===========================================================================
// GNEMoveOffset method definitions
// ===========================================================================

GNEMoveOffset::GNEMoveOffset() :
    x(0),
    y(0),
    z(0) {
}


GNEMoveOffset::GNEMoveOffset(const double x_, const double y_) :
    x(x_),
    y(y_),
    z(0) {
}


GNEMoveOffset::GNEMoveOffset(const double z_) :
    x(0),
    y(0),
    z(z_) {
}


GNEMoveOffset::~GNEMoveOffset() {}

// ===========================================================================
// GNEMoveResult method definitions
// ===========================================================================

GNEMoveResult::GNEMoveResult() :
    laneOffset(0),
    newStartPos(0),
    newEndPos(0),
    newLane(nullptr) {}


GNEMoveResult::~GNEMoveResult() {}

// ===========================================================================
// GNEMoveElement method definitions
// ===========================================================================

GNEMoveElement::GNEMoveElement() :
    myMoveElementLateralOffset(0) {
}


void
GNEMoveElement::moveElement(const GNEViewNet* viewNet, GNEMoveOperation* moveOperation, const GNEMoveOffset& offset) {
    // declare move result
    GNEMoveResult moveResult;
    // set geometry points to move
    moveResult.geometryPointsToMove = moveOperation->geometryPointsToMove;
    // check if we're moving over a lane shape, an entire shape or only certain geometry point
    if (moveOperation->firstLane) {
        // calculate movement over lane
        if (moveOperation->secondLane) {
            calculateDoubleMovementOverTwoLanes(moveResult, viewNet, moveOperation->firstLane, moveOperation->originalStarPos, moveOperation->secondLane, moveOperation->originalEndPos, offset);
        } else if (moveOperation->originalEndPos != INVALID_DOUBLE) {
            calculateDoubleMovementOverOneLane(moveResult, viewNet, moveOperation->firstLane, moveOperation->originalStarPos, moveOperation->originalEndPos, offset);
        } else {
            calculateSingleMovementOverOneLane(moveResult, viewNet, moveOperation->firstLane, moveOperation->originalStarPos, offset);
        }
        // calculate new lane
        calculateNewLane(moveResult, viewNet, moveOperation);
    } else if (moveOperation->geometryPointsToMove.empty()) {
        // set values in moveResult
        moveResult.shapeToUpdate = moveOperation->shapeToMove;
        // move entire shape
        for (auto& geometryPointIndex : moveResult.shapeToUpdate) {
            if (geometryPointIndex != Position::INVALID) {
                // add offset
                geometryPointIndex.add(offset.x, offset.y, offset.z);
                // apply snap to active grid
                geometryPointIndex = viewNet->snapToActiveGrid(geometryPointIndex);
            } else {
                throw ProcessError("trying to move an invalid position");
            }
        }
    } else {
        // set values in moveResult
        moveResult.shapeToUpdate = moveOperation->shapeToMove;
        // move geometry points
        for (const auto& geometryPointIndex : moveOperation->geometryPointsToMove) {
            if (moveResult.shapeToUpdate[geometryPointIndex] != Position::INVALID) {
                // add offset
                moveResult.shapeToUpdate[geometryPointIndex].add(offset.x, offset.y, offset.z);
                // apply snap to active grid
                moveResult.shapeToUpdate[geometryPointIndex] = viewNet->snapToActiveGrid(moveResult.shapeToUpdate[geometryPointIndex]);
            } else {
                throw ProcessError("trying to move an invalid position");
            }
        }
    }
    // move shape element
    moveOperation->moveElement->setMoveShape(moveResult);
}


void
GNEMoveElement::commitMove(const GNEViewNet* viewNet, GNEMoveOperation* moveOperation, const GNEMoveOffset& offset, GNEUndoList* undoList) {
    // declare move result
    GNEMoveResult moveResult;
    // check if we're moving over a lane shape, an entire shape or only certain geometry point
    if (moveOperation->firstLane) {
        // set geometry points to move
        moveResult.geometryPointsToMove = moveOperation->geometryPointsToMove;
        // restore original position over lanes
        PositionVector originalPosOverLanes;
        originalPosOverLanes.push_back(Position(moveOperation->originalStarPos, 0));
        if (moveOperation->originalEndPos != INVALID_DOUBLE) {
            originalPosOverLanes.push_back(Position(moveOperation->originalEndPos, 0));
        }
        // set shapeToUpdate with originalPosOverLanes
        moveResult.shapeToUpdate = originalPosOverLanes;
        // set originalPosOverLanes in element
        moveOperation->moveElement->setMoveShape(moveResult);
        // calculate movement over lane
        if (moveOperation->secondLane) {
            calculateDoubleMovementOverTwoLanes(moveResult, viewNet, moveOperation->firstLane, moveOperation->originalStarPos, moveOperation->secondLane, moveOperation->originalEndPos, offset);
        } else if (moveOperation->originalEndPos != INVALID_DOUBLE) {
            calculateDoubleMovementOverOneLane(moveResult, viewNet, moveOperation->firstLane, moveOperation->originalStarPos, moveOperation->originalEndPos, offset);
        } else {
            calculateSingleMovementOverOneLane(moveResult, viewNet, moveOperation->firstLane, moveOperation->originalStarPos, offset);
        }
        // calculate new lane
        calculateNewLane(moveResult, viewNet, moveOperation);
    } else {
        // set original geometry points to move
        moveResult.geometryPointsToMove = moveOperation->originalGeometryPoints;
        // set shapeToUpdate with originalPosOverLanes
        moveResult.shapeToUpdate = moveOperation->originalShape;
        // first restore original geometry geometry
        moveOperation->moveElement->setMoveShape(moveResult);
        // set new geometry points to move
        moveResult.geometryPointsToMove = moveOperation->geometryPointsToMove;
        // set values in moveResult
        moveResult.shapeToUpdate = moveOperation->shapeToMove;
        // check if we're moving an entire shape or  only certain geometry point
        if (moveOperation->geometryPointsToMove.empty()) {
            // move entire shape
            for (auto& geometryPointIndex : moveResult.shapeToUpdate) {
                if (geometryPointIndex != Position::INVALID) {
                    // add offset
                    geometryPointIndex.add(offset.x, offset.y, offset.z);
                    // apply snap to active grid
                    geometryPointIndex = viewNet->snapToActiveGrid(geometryPointIndex);
                } else {
                    throw ProcessError("trying to move an invalid position");
                }
            }
        } else {
            // only move certain geometry points
            for (const auto& geometryPointIndex : moveOperation->geometryPointsToMove) {
                if (moveResult.shapeToUpdate[geometryPointIndex] != Position::INVALID) {
                    // add offset
                    moveResult.shapeToUpdate[geometryPointIndex].add(offset.x, offset.y, offset.z);
                    // apply snap to active grid
                    moveResult.shapeToUpdate[geometryPointIndex] = viewNet->snapToActiveGrid(moveResult.shapeToUpdate[geometryPointIndex]);
                } else {
                    throw ProcessError("trying to move an invalid position");
                }
            }
            // remove double points (only in commitMove)
            if (moveResult.shapeToUpdate.size() > 2) {
                moveResult.shapeToUpdate.removeDoublePoints(2);
            }
        }
    }
    // commit move shape
    moveOperation->moveElement->commitMoveShape(moveResult, undoList);
}


void
GNEMoveElement::calculateSingleMovementOverOneLane(GNEMoveResult& moveResult, const GNEViewNet* viewNet, const GNELane* lane, const double pos, const GNEMoveOffset& offset) {
    // get lane length
    const double laneShapeLengt = lane->getLaneShape().length2D();
    // declare position over lane offset
    double posOverLaneOffset = 0;
    // calculate position at offset
    Position lanePosition = lane->getLaneShape().positionAtOffset2D(pos);
    // apply offset to positionAtCentralPosition
    lanePosition.add(offset.x, offset.y, offset.z);
    // snap to grid
    lanePosition = viewNet->snapToActiveGrid(lanePosition);
    // calculate new posOverLane perpendicular
    const double newPosOverLanePerpendicular = lane->getLaneShape().nearest_offset_to_point2D(lanePosition);
    // calculate posOverLaneOffset
    if (newPosOverLanePerpendicular == -1) {
        // calculate new posOverLane non-perpendicular
        const double newPosOverLane = lane->getLaneShape().nearest_offset_to_point2D(lanePosition, false);
        // out of lane shape, then place element in lane extremes
        if (newPosOverLane == 0) {
            posOverLaneOffset = pos;
        } else {
            posOverLaneOffset = pos - laneShapeLengt;
        }
    } else {
        // within of lane shape
        if (newPosOverLanePerpendicular < 0) {
            posOverLaneOffset = pos;
        } else if (newPosOverLanePerpendicular > laneShapeLengt) {
            posOverLaneOffset = laneShapeLengt;
        } else {
            posOverLaneOffset = pos - newPosOverLanePerpendicular;
        }
    }
    // update moveResult
    moveResult.newStartPos = (pos - posOverLaneOffset) / lane->getLengthGeometryFactor();
}


void
GNEMoveElement::calculateDoubleMovementOverOneLane(GNEMoveResult& moveResult, const GNEViewNet* viewNet, const GNELane* lane, const double startPos, const double endPos, const GNEMoveOffset& offset) {
    // calculate lenght between pos over lanes
    const double centralPosition = (startPos + endPos) * 0.5;
    // calculate middle lenght between first and last pos over lanes
    const double middleLenght = std::abs(endPos - startPos) * 0.5;
    // get lane length
    const double laneShapeLengt = lane->getLaneShape().length2D();
    // declare position over lane offset
    double posOverLaneOffset = 0;
    // calculate position at offset given by centralPosition
    Position lanePositionAtCentralPosition = lane->getLaneShape().positionAtOffset2D(centralPosition);
    // apply offset to positionAtCentralPosition
    lanePositionAtCentralPosition.add(offset.x, offset.y, offset.z);
    // snap to grid
    lanePositionAtCentralPosition = viewNet->snapToActiveGrid(lanePositionAtCentralPosition);
    // calculate new posOverLane perpendicular
    const double newPosOverLanePerpendicular = lane->getLaneShape().nearest_offset_to_point2D(lanePositionAtCentralPosition);
    // calculate posOverLaneOffset
    if (newPosOverLanePerpendicular == -1) {
        // calculate new posOverLane non-perpendicular
        const double newPosOverLane = lane->getLaneShape().nearest_offset_to_point2D(lanePositionAtCentralPosition, false);
        // out of lane shape, then place element in lane extremes
        if (newPosOverLane == 0) {
            posOverLaneOffset = startPos;
        } else {
            posOverLaneOffset = endPos - laneShapeLengt;
        }
    } else {
        // within of lane shape
        if ((newPosOverLanePerpendicular - middleLenght) < 0) {
            posOverLaneOffset = startPos;
        } else if ((newPosOverLanePerpendicular + middleLenght) > laneShapeLengt) {
            posOverLaneOffset = endPos - laneShapeLengt;
        } else {
            posOverLaneOffset = centralPosition - newPosOverLanePerpendicular;
        }
    }
    // update moveResult
    moveResult.newStartPos = (startPos - posOverLaneOffset) / lane->getLengthGeometryFactor();
    moveResult.newEndPos = (endPos - posOverLaneOffset) / lane->getLengthGeometryFactor();
}


void 
GNEMoveElement::calculateDoubleMovementOverTwoLanes(GNEMoveResult& moveResult, const GNEViewNet* viewNet, const GNELane* firstLane, const double startPos, const GNELane* secondLane, const double endPos, const GNEMoveOffset& offset) {

}


void
GNEMoveElement::calculateNewLane(GNEMoveResult& moveResult, const GNEViewNet* viewNet, const GNEMoveOperation* moveOperation) {
    // first check if change lane is allowed
    if (moveOperation->allowChangeLane) {
        // get cursor position
        const Position cursorPosition = viewNet->getPositionInformation();
        // iterate over edge lanes
        for (const auto& lane : moveOperation->firstLane->getParentEdge()->getLanes()) {
            // avoid moveOperation lane
            if (lane != moveOperation->firstLane) {
                // calculate offset over lane shape
                const double offSet = lane->getLaneShape().nearest_offset_to_point2D(cursorPosition, true);
                // calculate position over lane shape
                const Position posOverLane = lane->getLaneShape().positionAtOffset2D(offSet);
                // check distance
                if (posOverLane.distanceSquaredTo2D(cursorPosition) < 1) {
                    // update newlane
                    moveResult.newLane = lane;
                    // calculate offset over moveOperation lane
                    const double offsetMoveOperationLane = moveOperation->firstLane->getLaneShape().nearest_offset_to_point2D(cursorPosition, true);
                    // calculate position over moveOperation lane
                    const Position posOverMoveOperationLane = moveOperation->firstLane->getLaneShape().positionAtOffset2D(offsetMoveOperationLane);
                    // update moveResult of laneOffset
                    moveResult.laneOffset = posOverLane.distanceTo2D(posOverMoveOperationLane);
                    // change sign of  moveResult laneOffset depending of lane index
                    if (moveOperation->firstLane->getIndex() < moveResult.newLane->getIndex()) {
                        moveResult.laneOffset *= -1;
                    }
                }
            }
        }
    } else {
        // reset values
        moveResult.newLane = nullptr;
        moveResult.laneOffset = 0;
    }
}

/****************************************************************************/
