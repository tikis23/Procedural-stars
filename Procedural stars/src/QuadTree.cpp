#include "QuadTree.h"
#include "FrustumCulling.h"
#include <format>
#define NORTH_WEST 0
#define NORTH_EAST 1
#define SOUTH_EAST 2
#define SOUTH_WEST 3

#define MIRROR(s) (((s) + 2) % 4)
#define ADJACENT(s, q) ((4 + (q) - (s)) % 4 <= 1)
#define REFLECT(s, q) ((s) % 2 ? ((q) % 2 ? (q) - 1 : (q) + 1) : 3 - (q))

QuadTreeNode::QuadTreeNode() {
    m_splitDistance = 4;
}

QuadTreeNode::~QuadTreeNode() {
    Merge();
    // Disconnect neighbors
    for (unsigned int i = 0; i < 4; i++)
        if (m_neighbor[i])
            m_neighbor[i]->SetNeighbor(MirrorSide(i), 0);

    // Delete the patch
    delete m_patch;
}

void QuadTreeNode::Update(glm::dvec3 cameraPos) {
    if (m_patch == nullptr) {
        m_patch = new TerrainPatch(this);
        m_patch->GetBoundingSphere(m_boundingSphereCenter, m_boundingSphereRadius);
    }

    double cameraDist = glm::length(cameraPos - m_boundingSphereCenter);
    bool split = cameraDist < m_boundingSphereRadius * m_splitDistance;

    if (!split) {
        Merge();
        return;
    }

    if (!IsLeaf() || Split())
        for (int i = 0; i < 4; i++)
            m_child[i]->Update(cameraPos);
}

void QuadTreeNode::Render(Shader* shader) {
    if (!FrustumCulling::CheckSphere(m_boundingSphereCenter, m_boundingSphereRadius))
        return;

    if (!IsLeaf()) {
        for (int i = 0; i < 4; i++)
            m_child[i]->Render(shader);
        return;
    }
    m_patch->Render(shader);
}

bool QuadTreeNode::Split() {
    if (!IsLeaf() || m_depth + 1 > MAX_DEPTH || sm_splitsLeft == 0)
        return false;
    sm_splitsLeft--;
    for (int i = 0; i < 4; i++) {
        m_child[i] = new QuadTreeNode;
        m_child[i]->m_parent = this;
        m_child[i]->m_quadrant = i;
        m_child[i]->m_face = m_face;
        m_child[i]->m_patchRotation = m_patchRotation;
        m_child[i]->m_depth = m_depth + 1;
        m_child[i]->m_size = 1.0 / (1 << m_child[i]->m_depth);
        if (i == NORTH_WEST)
            m_child[i]->m_position = { m_position.x - m_child[i]->m_size, m_position.y + m_child[i]->m_size };
        else if (i == NORTH_EAST)
            m_child[i]->m_position = { m_position.x + m_child[i]->m_size, m_position.y + m_child[i]->m_size };
        else if (i == SOUTH_EAST)
            m_child[i]->m_position = { m_position.x + m_child[i]->m_size, m_position.y - m_child[i]->m_size };
        else if (i == SOUTH_WEST)
            m_child[i]->m_position = { m_position.x - m_child[i]->m_size, m_position.y - m_child[i]->m_size };
    }

    // Connect the children
    //m_child[NORTH_WEST]->SetNeighbor(EAST , m_child[NORTH_EAST]);
    //m_child[NORTH_EAST]->SetNeighbor(SOUTH, m_child[SOUTH_EAST]);
    //m_child[SOUTH_EAST]->SetNeighbor(WEST , m_child[SOUTH_WEST]);
    //m_child[SOUTH_WEST]->SetNeighbor(NORTH, m_child[NORTH_WEST]);


    // Connect neighbors
    //for (int i = 0; i < 4; i++)
    //    if (m_neighbor[i] && !m_neighbor[i]->IsLeaf())
    //        m_neighbor[i]->FindNeighbor(MirrorSide(i));

    return true;
}

void QuadTreeNode::Merge() {
    if (IsLeaf())
        return;
    for (int i = 0; i < 4; i++) {
        delete m_child[i];
        m_child[i] = nullptr;
    }
}

bool QuadTreeNode::IsLeaf()const {
    if (m_child[0] == nullptr)
        return true;
    return false;
}


unsigned int QuadTreeNode::MirrorSide(const unsigned int& side) {
    return NORTH;
    // If no neighbor; use default mirroring
    if (!m_neighbor[side])
        return MIRROR(side);

    // Get source and destination faces
    const unsigned int f0 = m_face;
    const unsigned int f1 = m_neighbor[side]->m_face;

    // If within the same face or faces with equal properties
    if (f0 == f1)
        return MIRROR(side);

    {
        //// Source face
        //switch (f0)
        //{
        //case FACE_TOP:
        //    return MIRROR(side);
        //    break;

        //case FACE_BACK:
        //    switch (f1)
        //    {
        //    case FACE_TOP:
        //    case FACE_BOTTOM:
        //        return MIRROR(side);
        //        break;
        //    case FACE_LEFT:
        //    case FACE_RIGHT:
        //        return NORTH;
        //        break;
        //    }
        //    break;

        //case FACE_BOTTOM:
        //    switch (f1)
        //    {
        //    case FACE_BACK:
        //    case FACE_FRONT:
        //        return MIRROR(side);
        //        break;
        //    case FACE_LEFT:
        //        return WEST;
        //        break;
        //    case FACE_RIGHT:
        //        return EAST;
        //        break;
        //    }
        //    break;

        //case FACE_FRONT:
        //    switch (f1)
        //    {
        //    case FACE_BOTTOM:
        //    case FACE_TOP:
        //        return MIRROR(side);
        //        break;
        //    case FACE_LEFT:
        //    case FACE_RIGHT:
        //        return SOUTH;
        //        break;
        //    }
        //    break;

        //case FACE_RIGHT:
        //    switch (f1)
        //    {
        //    case FACE_TOP:
        //        return MIRROR(side);
        //        break;
        //    case FACE_BACK:
        //        return EAST;
        //        break;
        //    case FACE_BOTTOM:
        //        return EAST;
        //        break;
        //    case FACE_FRONT:
        //        return EAST;
        //        break;
        //    }
        //    break;
        //case FACE_LEFT:
        //    switch (f1)
        //    {
        //    case FACE_TOP:
        //        return MIRROR(side);
        //        break;
        //    case FACE_BACK:
        //        return WEST;
        //        break;
        //    case FACE_BOTTOM:
        //        return WEST;
        //        break;
        //    case FACE_FRONT:
        //        return WEST;
        //        break;
        //    }
        //    break;
        //}
    }

    // Source face
    switch (f0)
    {
    case FACE_TOP:
        return MIRROR(side);
        break;

    case FACE_BACK:
        switch (f1)
        {
        case FACE_TOP:
        case FACE_BOTTOM:
            return MIRROR(side);
            break;
        case FACE_LEFT:
        case FACE_RIGHT:
            return NORTH;
            break;
        }
        break;

    case FACE_BOTTOM:
        switch (f1)
        {
        case FACE_BACK:
        case FACE_FRONT:
            return MIRROR(side);
            break;
        case FACE_LEFT:
            return WEST;
            break;
        case FACE_RIGHT:
            return EAST;
            break;
        }
        break;

    case FACE_FRONT:
        switch (f1)
        {
        case FACE_BOTTOM:
        case FACE_TOP:
            return MIRROR(side);
            break;
        case FACE_LEFT:
        case FACE_RIGHT:
            return SOUTH;
            break;
        }
        break;

    case FACE_RIGHT:
        switch (f1)
        {
        case FACE_TOP:
            return MIRROR(side);
            break;
        case FACE_BACK:
            return EAST;
            break;
        case FACE_BOTTOM:
            return EAST;
            break;
        case FACE_FRONT:
            return EAST;
            break;
        }
        break;
    case FACE_LEFT:
        switch (f1)
        {
        case FACE_TOP:
            return MIRROR(side);
            break;
        case FACE_BACK:
            return WEST;
            break;
        case FACE_BOTTOM:
            return WEST;
            break;
        case FACE_FRONT:
            return WEST;
            break;
        }
        break;
    }
    return MIRROR(side);
}

unsigned int QuadTreeNode::MirrorQuadrant(const unsigned int& side, const unsigned int& quadrant) {
    // If mirroring within the parent node
    if (!ADJACENT(side, quadrant))
        return REFLECT(side, quadrant);

    // If no parent or parent neighbor
    if (!m_parent || !m_parent->m_neighbor[side])
        return REFLECT(side, quadrant);

    // Get source and destination faces
    const unsigned int f0 = m_face;
    const unsigned int f1 = m_parent->m_neighbor[side]->m_face;

    // If within the same face or faces with equal properties
    if (f0 == f1)
        return REFLECT(side, quadrant);

    // Source face
    switch (f0)
    {
    case FACE_TOP:
        REFLECT(side, quadrant);
        break;
    case FACE_BACK:
        switch (f1)
        {
        case FACE_TOP:
        case FACE_BOTTOM:
            REFLECT(side, quadrant);
            break;
        case FACE_LEFT:
        case FACE_RIGHT:
            return (quadrant + 1) % 4;
            break;
        }
        break;
    case FACE_BOTTOM:
        switch (f1)
        {
        case FACE_BACK:
        case FACE_FRONT:
            REFLECT(side, quadrant);
            break;
        case FACE_LEFT:
        case FACE_RIGHT:
            return (quadrant + 2) % 4;
            break;
        }
        break;
    case FACE_FRONT:
        switch (f1)
        {
        case FACE_BOTTOM:
        case FACE_TOP:
            REFLECT(side, quadrant);
            break;
        case FACE_LEFT:
        case FACE_RIGHT:
            return (quadrant + 3) % 4;
            break;
        }
        break;

    case FACE_LEFT:
    case FACE_RIGHT:
        switch (f1)
        {
        case FACE_TOP:
            REFLECT(side, quadrant);
            break;
        case FACE_BACK:
            return (quadrant + 1) % 4;
            break;
        case FACE_BOTTOM:
            return (quadrant + 2) % 4;
            break;
        case FACE_FRONT:
            return (quadrant + 3) % 4;
            break;
        }
        break;
    }

    return REFLECT(side, quadrant);
}

void QuadTreeNode::SetNeighbor(unsigned int side, QuadTreeNode* neighbor) {
    m_neighbor[side] = neighbor;
    if (neighbor) {
        const unsigned int sideMirrored = MirrorSide(side);
        neighbor->m_neighbor[sideMirrored] = this;
        neighbor->UpdateNeighborDetail(sideMirrored);
    }
    UpdateNeighborDetail(side);
}

QuadTreeNode* QuadTreeNode::FindEqualOrHigherNeighbor(unsigned int side) {
    // Find the youngest ancestor with a neighbor in the given direction
    for (const QuadTreeNode* node = this; node != 0; node = node->m_parent) {
        if (node->m_neighbor[side])
            return node->m_neighbor[side];
    }
    return nullptr;
}

void QuadTreeNode::FindNeighbor(const unsigned int side)
{
    // If the current node has no neighbor in the given direction, but its parent does
    if (!m_neighbor[side] && m_parent && m_parent->m_neighbor[side]) {
        // If a valid neighbor is found (child of the parent's neighbor); use it
        if (QuadTreeNode* neighbor = m_parent->m_neighbor[side]->m_child[MirrorQuadrant(side, m_quadrant)])
            SetNeighbor(side, neighbor);
        else
            return;
    }

    // If no leaf node; find child node neighbors
    if (!IsLeaf())
        for (unsigned int i = 0; i < 4; i++)
            if (ADJACENT(side, i))
                m_child[i]->FindNeighbor(side);
}

void QuadTreeNode::UpdateNeighborDetail(unsigned int side) {
    // Update neighbor detail differences
    for (unsigned int i = 0; i < 4; i++) {
        QuadTreeNode* neighbor = FindEqualOrHigherNeighbor(i);
        if (neighbor != nullptr)
            m_neighborDetailDifferences[i] = std::min(m_depth - neighbor->m_depth, (unsigned int)MAX_DETAIL_DIFFERENCE);
    }
    // Force child nodes on the updated side to redraw
    if (!IsLeaf())
        for (unsigned int i = 0; i < 4; i++)
            if (ADJACENT(side, i))
                m_child[i]->UpdateNeighborDetail(side);
}

double QuadTreeNode::m_splitDistance;
int QuadTreeNode::sm_splitsLeft;