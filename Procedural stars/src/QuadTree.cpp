#include "QuadTree.h"

QuadTreeNode::QuadTreeNode() {
}

QuadTreeNode::~QuadTreeNode() {
}

void QuadTreeNode::Split() {
}

void QuadTreeNode::Merge() {
}

void QuadTreeNode::Update() {
    if (m_patch == nullptr) {
        m_patch = new TerrainPatch(this);
        m_patch->GetBoundingSphere(m_boundingSphereCenter, m_boundingSphereRadius);
    }

    //// Use the distance from the camera to the bounding sphere to calculate the split distance
    //const double distanceToCamera = (face->GetTerrain()->GetCameraPositionLocal() - boundingSphereCenter).GetLength();
    //const bool split = distanceToCamera < boundingSphereRadius* splitDistanceScale;

    //// If the node shouldn't be split
    //if (!split)
    //{
    //    // If not a leaf node; merge
    //    if (!IsLeaf())
    //        Merge();
    //    return;
    //}

    //// If the node has children, or if children are successfully created; update the children
    //if (!IsLeaf() || Split())
    //    for (unsigned int i = 0; i < 4; i++)
    //        children[i]->Update();
}