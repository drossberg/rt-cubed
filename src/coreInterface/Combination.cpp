/*                      C O M B I N A T I O N . C P P
 * BRL-CAD
 *
 * Copyright (c) 2008-2009 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file Combination.cpp
 *
 *  BRL-CAD core C++ interface:
 *      combination (ID_COMBINATION) database object implementation
 *
 *  Origin -
 *      TNO (Netherlands)
 *      IABG mbH (Germany)
 */

#include <cassert>

#include "raytrace.h"

#include <brlcad/Combination.h>


using namespace BRLCAD;


static Combination::ConstTreeNode::Operator ConvertOperator
(
    tree* brlcadTree
) {
    Combination::ConstTreeNode::Operator ret = Combination::ConstTreeNode::Null;

    if (brlcadTree != 0) {
        switch (brlcadTree->tr_op) {
        case OP_UNION:
            ret = Combination::ConstTreeNode::Union;
            break;

        case OP_INTERSECT:
            ret = Combination::ConstTreeNode::Intersection;
            break;

        case OP_SUBTRACT:
            ret = Combination::ConstTreeNode::Subtraction;
            break;

        case OP_XOR:
            ret = Combination::ConstTreeNode::ExclusiveOr;
            break;

        case OP_NOT:
            ret = Combination::ConstTreeNode::Not;
            break;

        case OP_DB_LEAF:
            ret = Combination::ConstTreeNode::Leaf;
            break;

        default:
            assert(0);
        }
    }

    return ret;
}


static int ConvertOperatorBack
(
    Combination::ConstTreeNode::Operator op
) {
    int ret = 0;

    switch (op) {
    case Combination::ConstTreeNode::Union:
        ret = OP_UNION;
        break;

    case Combination::ConstTreeNode::Intersection:
        ret = OP_INTERSECT;
        break;

    case Combination::ConstTreeNode::Subtraction:
        ret = OP_SUBTRACT;
        break;

    case Combination::ConstTreeNode::ExclusiveOr:
        ret = OP_XOR;
        break;

    case Combination::ConstTreeNode::Not:
        ret = OP_NOT;
        break;

    case Combination::ConstTreeNode::Leaf:
        ret = OP_DB_LEAF;
        break;

    default:
        assert(0);
    }

    return ret;
}


static tree* ParentTree
(
    tree* searchTree,
    tree* rootTree
) {
    tree* ret = 0;

    switch (ConvertOperator(rootTree)) {
    case Combination::ConstTreeNode::Union:
    case Combination::ConstTreeNode::Intersection:
    case Combination::ConstTreeNode::Subtraction:
    case Combination::ConstTreeNode::ExclusiveOr:
        if (searchTree == rootTree->tr_b.tb_left)
            ret = rootTree;
        else if (searchTree == rootTree->tr_b.tb_right)
            ret = rootTree;
        else {
            ret = ParentTree(searchTree, rootTree->tr_b.tb_left);

            if (ret == 0)
                ParentTree(searchTree, rootTree->tr_b.tb_right);
        }
        break;

    case Combination::ConstTreeNode::Not:
        if (searchTree == rootTree->tr_b.tb_left)
            ret = rootTree;
        else 
            ret = ParentTree(searchTree, rootTree->tr_b.tb_left);
        break;

    default:
        assert(0);
    }

    return ret;
}


//
// class Combination::ConstTreeNode
//

Combination::ConstTreeNode::Operator Combination::ConstTreeNode::Operation(void) const {
    return ConvertOperator(m_tree);
}


Combination::ConstTreeNode Combination::ConstTreeNode::LeftOperand(void) const {
    ConstTreeNode ret;

    switch (ConvertOperator(m_tree)) {
    case Union:
    case Intersection:
    case Subtraction:
    case ExclusiveOr:
        ret.m_tree = m_tree->tr_b.tb_left;
        break;

    default:
        assert(0);
    }

    return ret;
}


Combination::ConstTreeNode Combination::ConstTreeNode::RightOperand(void) const {
    ConstTreeNode ret;

    switch (ConvertOperator(m_tree)) {
    case Union:
    case Intersection:
    case Subtraction:
    case ExclusiveOr:
        ret.m_tree = m_tree->tr_b.tb_right;
        break;

    default:
        assert(0);
    }

    return ret;
}


Combination::ConstTreeNode Combination::ConstTreeNode::Operand(void) const {
    ConstTreeNode ret;

    switch (ConvertOperator(m_tree)) {
    case Not:
        ret.m_tree = m_tree->tr_b.tb_left;
        break;

    default:
        assert(0);
    }

    return ret;
}


const char* Combination::ConstTreeNode::Name(void) const {
    const char* ret = 0;

    switch (ConvertOperator(m_tree)) {
    case Leaf:
        ret = m_tree->tr_l.tl_name;
        break;

    default:
        assert(0);
    }

    return ret;
}


const double* Combination::ConstTreeNode::Matrix(void) const {
    const double* ret = 0;

    switch (ConvertOperator(m_tree)) {
    case Leaf:
        ret = m_tree->tr_l.tl_mat;
        break;

    default:
        assert(0);
    }

    return ret;
}


//
// class Combination::TreeNode
//

Combination::TreeNode Combination::TreeNode::LeftOperand(void) {
    TreeNode ret;

    switch (ConvertOperator(m_tree)) {
    case Union:
    case Intersection:
    case Subtraction:
    case ExclusiveOr:
        ret.m_tree      = m_tree->tr_b.tb_left;
        ret.m_internalp = m_internalp;
        ret.m_resp      = m_resp;
        break;

    default:
        assert(0);
    }

    return ret;
}


Combination::TreeNode Combination::TreeNode::RightOperand(void) {
    TreeNode ret;

    switch (ConvertOperator(m_tree)) {
    case Union:
    case Intersection:
    case Subtraction:
    case ExclusiveOr:
        ret.m_tree      = m_tree->tr_b.tb_right;
        ret.m_internalp = m_internalp;
        ret.m_resp      = m_resp;
        break;

    default:
        assert(0);
    }

    return ret;
}


Combination::TreeNode Combination::TreeNode::Operand(void) {
    TreeNode ret;

    switch (ConvertOperator(m_tree)) {
    case Not:
        ret.m_tree      = m_tree->tr_b.tb_left;
        ret.m_internalp = m_internalp;
        ret.m_resp      = m_resp;
        break;

    default:
        assert(0);
    }

    return ret;
}


Combination::TreeNode Combination::TreeNode::Apply
(
    Combination::ConstTreeNode::Operator op
) {
    assert(m_tree != 0);

    TreeNode ret;

    if (BU_SETJUMP)
        goto END_MARK;

    if (m_tree != 0) {
        switch (op) {
        case Not:
            tree* newNode;
            RT_GET_TREE(newNode, m_resp);

            *newNode = *m_tree; // copies all values in the union

            // this->m_tree will be transfered to ret and becomes the "Not" node
            ret.m_tree               = m_tree;
            ret.m_tree->tr_op        = OP_NOT;
            ret.m_tree->tr_b.tb_left = newNode;

            ret.m_internalp = m_internalp;
            ret.m_resp      = m_resp;

            // this gets newNode (which is a copy of the former this->m_tree)
            m_tree = newNode;
            break;

        default:
            assert(0);
        }
    }

END_MARK:
    BU_UNSETJUMP;

    return ret;
}


Combination::TreeNode Combination::TreeNode::Apply
(
    Combination::ConstTreeNode::Operator op,
    const Combination::ConstTreeNode&    theOther
) {
    assert(m_tree != 0);
    assert(theOther.m_tree != 0);

    TreeNode ret;

    if (BU_SETJUMP)
        goto END_MARK;

    if ((m_tree != 0) && (theOther.m_tree != 0)) {
        switch (op) {
        case Union:
        case Intersection:
        case Subtraction:
        case ExclusiveOr:
            tree* newNode;
            RT_GET_TREE(newNode, m_resp);

            *newNode = *m_tree; // copies all values in the union

            // this->m_tree will be transfered to ret and becomes the <op> node
            ret.m_tree                = m_tree;
            ret.m_tree->tr_op         = ConvertOperatorBack(op);
            ret.m_tree->tr_b.tb_left  = newNode;
            ret.m_tree->tr_b.tb_right = db_dup_subtree(theOther.m_tree, m_resp);

            ret.m_internalp = m_internalp;
            ret.m_resp      = m_resp;

            // this gets newNode (which is a copy of the former this->m_tree)
            m_tree = newNode;
            break;

        default:
            assert(0);
        }
    }

END_MARK:
    BU_UNSETJUMP;

    return ret;
}


Combination::TreeNode Combination::TreeNode::Apply
(
    Combination::ConstTreeNode::Operator op,
    const char*                          leafName
) {
    assert(leafName != 0);

    TreeNode ret;

    if (BU_SETJUMP)
        goto END_MARK;

    if (leafName != 0) {
        TreeNode newLeaf(*this);
        RT_GET_TREE(newLeaf.m_tree, newLeaf.m_resp);

        newLeaf.m_tree->magic        = RT_TREE_MAGIC;
        newLeaf.m_tree->tr_op        = OP_DB_LEAF;
        newLeaf.m_tree->tr_l.tl_mat  = 0;
        newLeaf.m_tree->tr_l.tl_name = bu_strdup(leafName);

        ret = Apply(op, newLeaf);

        if (ret.m_tree == 0) // in case of an error
            RT_FREE_TREE(newLeaf.m_tree, newLeaf.m_resp);
    }

END_MARK:
    BU_UNSETJUMP;

    return ret;
}


Combination::TreeNode Combination::TreeNode::Apply
(
    const Combination::ConstTreeNode&    theOther,
    Combination::ConstTreeNode::Operator op
) {
    assert(m_tree != 0);
    assert(theOther.m_tree != 0);

    TreeNode ret;

    if (BU_SETJUMP)
        goto END_MARK;

    if ((m_tree != 0) && (theOther.m_tree != 0)) {
        switch (op) {
        case Union:
        case Intersection:
        case Subtraction:
        case ExclusiveOr:
            tree* newNode;
            RT_GET_TREE(newNode, m_resp);

            *newNode = *m_tree; // copies all values in the union

            // this->m_tree will be transfered to ret and becomes the <op> node
            ret.m_tree                = m_tree;
            ret.m_tree->tr_op         = ConvertOperatorBack(op);
            ret.m_tree->tr_b.tb_left  = db_dup_subtree(theOther.m_tree, m_resp);
            ret.m_tree->tr_b.tb_right = newNode;

            ret.m_internalp = m_internalp;
            ret.m_resp      = m_resp;

            // this gets newNode (which is a copy of the former this->m_tree)
            m_tree = newNode;
            break;

        default:
            assert(0);
        }
    }

END_MARK:
    BU_UNSETJUMP;

    return ret;
}


Combination::TreeNode Combination::TreeNode::Apply
(
    const char*                          leafName,
    Combination::ConstTreeNode::Operator op
) {
    assert(leafName != 0);

    TreeNode ret;

    if (BU_SETJUMP)
        goto END_MARK;

    if (leafName != 0) {
        TreeNode newLeaf(*this);
        RT_GET_TREE(newLeaf.m_tree, newLeaf.m_resp);

        newLeaf.m_tree->magic        = RT_TREE_MAGIC;
        newLeaf.m_tree->tr_op        = OP_DB_LEAF;
        newLeaf.m_tree->tr_l.tl_mat  = 0;
        newLeaf.m_tree->tr_l.tl_name = bu_strdup(leafName);

        ret = Apply(newLeaf, op);

        if (ret.m_tree == 0) // in case of an error
            RT_FREE_TREE(newLeaf.m_tree, newLeaf.m_resp);
    }

END_MARK:
    BU_UNSETJUMP;

    return ret;
}


void Combination::TreeNode::Delete(void) {
    assert(m_tree != 0);

    TreeNode ret;

    if (BU_SETJUMP)
        goto END_MARK;

    while (m_tree != 0) {
        if (m_tree == m_internalp->tree) {
            db_free_tree(m_tree, m_resp);
            m_internalp->tree = 0;

            m_tree      = 0;
            m_internalp = 0;
            m_resp      = 0;
        }
        else {
            tree* parent = ParentTree(m_tree, m_internalp->tree);

            if (parent != 0) {
                switch (ConvertOperator(parent)) {
                case Union:
                case Intersection:
                case Subtraction:
                case ExclusiveOr:
                    if (m_tree == parent->tr_b.tb_left)
                        db_tree_del_lhs(parent, m_resp);
                    else {
                        assert(m_tree == parent->tr_b.tb_right);
                        db_tree_del_rhs(parent, m_resp);
                    }

                    m_tree      = 0;
                    m_internalp = 0;
                    m_resp      = 0;
                    break;

                case Not:
                    m_tree = parent; // go into the next iteration
                    break;

                default:
                    assert(0);

                    // give up
                    m_tree      = 0;
                    m_internalp = 0;
                    m_resp      = 0;
                }
            }
            else {
                assert(0); // we have a problem here: m_tree is not a child of m_internalp->tree, give up

                m_tree      = 0;
                m_internalp = 0;
                m_resp      = 0;
            }
        }
    }

END_MARK:
    BU_UNSETJUMP;
}


//
// class Combination
//

Combination::Combination(void) throw() : Object() {
    m_internalp = static_cast<rt_comb_internal*>(bu_calloc(1, sizeof(rt_comb_internal), "BRLCAD::Combination::Combination::m_internalp"));

    m_internalp->magic = RT_COMB_MAGIC;
    bu_vls_init(&m_internalp->shader);
    bu_vls_init(&m_internalp->material);
    m_internalp->temperature = -1.;
}


Combination::Combination
(
    const Combination& original
) throw() : Object(original) {
    const rt_comb_internal* internalFrom = original.Internal();

    m_internalp = static_cast<rt_comb_internal*>(bu_calloc(1, sizeof(rt_comb_internal), "BRLCAD::Combination::Combination::m_internalp"));

    m_internalp->magic = internalFrom->magic;

    if (internalFrom->tree != 0)
        m_internalp->tree = db_dup_subtree(internalFrom->tree, m_resp);

    m_internalp->region_flag = internalFrom->region_flag;
    m_internalp->is_fastgen  = internalFrom->is_fastgen;
    m_internalp->region_id   = internalFrom->region_id;
    m_internalp->aircode     = internalFrom->aircode;
    m_internalp->GIFTmater   = internalFrom->GIFTmater;
    m_internalp->los         = internalFrom->los;
    m_internalp->rgb_valid   = internalFrom->rgb_valid;
    m_internalp->rgb[0]      = internalFrom->rgb[0];
    m_internalp->rgb[1]      = internalFrom->rgb[1];
    m_internalp->rgb[2]      = internalFrom->rgb[2];
    m_internalp->temperature = internalFrom->temperature;
    bu_vls_strcpy(&m_internalp->shader, bu_vls_addr(&internalFrom->shader));
    bu_vls_strcpy(&m_internalp->material, bu_vls_addr(&internalFrom->material));
    m_internalp->inherit     = internalFrom->inherit;
}


Combination::~Combination(void) throw() {
    if (m_internalp != 0) {
        if (m_internalp->tree != 0)
            db_free_tree(m_internalp->tree, m_resp);

        bu_vls_free(&m_internalp->shader);
        bu_vls_free(&m_internalp->material);

        bu_free(m_internalp, "BRLCAD::Combination::~Combination::m_internalp");
    }
}


const Combination& Combination::operator=(const Combination& original) throw() {
    if (&original != this) {
        Copy(original);

        const rt_comb_internal* internalFrom = original.Internal();
        rt_comb_internal*       internalTo = Internal();

        if (internalTo->tree != 0)
            db_free_tree(m_internalp->tree, m_resp);

        if (internalFrom->tree != 0)
            internalTo->tree = db_dup_subtree(internalFrom->tree, m_resp);
        else
            internalTo->tree = 0;

        internalTo->region_flag = internalFrom->region_flag;
        internalTo->is_fastgen  = internalFrom->is_fastgen;
        internalTo->region_id   = internalFrom->region_id;
        internalTo->aircode     = internalFrom->aircode;
        internalTo->GIFTmater   = internalFrom->GIFTmater;
        internalTo->los         = internalFrom->los;
        internalTo->rgb_valid   = internalFrom->rgb_valid;
        internalTo->rgb[0]      = internalFrom->rgb[0];
        internalTo->rgb[1]      = internalFrom->rgb[1];
        internalTo->rgb[2]      = internalFrom->rgb[2];
        internalTo->temperature = internalFrom->temperature;
        bu_vls_strcpy(&internalTo->shader, bu_vls_addr(&internalFrom->shader));
        bu_vls_strcpy(&internalTo->material, bu_vls_addr(&internalFrom->material));
        internalTo->inherit     = internalFrom->inherit;
    }

    return *this;
}


Combination::ConstTreeNode Combination::Tree(void) const {
    return ConstTreeNode(Internal()->tree);
}


Combination::TreeNode Combination::Tree(void) {
    return TreeNode(Internal()->tree, Internal(), m_resp);
}


void Combination::AddLeaf
(
    const char* leafName
) {
    rt_comb_internal* internalp = Internal();

    if (internalp->tree == 0) {
        if (!BU_SETJUMP) {
            RT_GET_TREE(internalp->tree, m_resp);

            internalp->tree->magic        = RT_TREE_MAGIC;
            internalp->tree->tr_op        = OP_DB_LEAF;
            internalp->tree->tr_l.tl_mat  = 0;
            internalp->tree->tr_l.tl_name = bu_strdup(leafName);
        }

        BU_UNSETJUMP;
    }
    else {
        Tree().Apply(ConstTreeNode::Union, leafName);
    }
}


bool Combination::IsRegion(void) const throw() {
    return Internal()->region_flag != 0;
}


void Combination::SetIsRegion(bool value) throw() {
    Internal()->region_flag = (value) ? '\1' : '\0';
}


Combination::FastgenType Combination::FastgenRegion(void) const throw() {
    Combination::FastgenType ret = Non;

    switch (Internal()->is_fastgen) {
    case REGION_NON_FASTGEN:
        ret = Non;
        break;

    case REGION_FASTGEN_PLATE:
        ret = Plate;
        break;

    case REGION_FASTGEN_VOLUME:
        ret = Volume;
        break;

    default:
        assert(0);
    }

    return ret;
}


void Combination::SetFastgenRegion(Combination::FastgenType value) throw() {
    switch (value) {
    case Non:
        Internal()->is_fastgen = REGION_NON_FASTGEN;
        break;

    case Plate:
        Internal()->is_fastgen = REGION_FASTGEN_PLATE;
        break;

    case Volume:
        Internal()->is_fastgen = REGION_FASTGEN_VOLUME;
        break;

    default:
        assert(0);
    }
}


int Combination::RegionId(void) const throw() {
    return Internal()->region_id;
}


void Combination:: SetRegionId(int value) throw() {
    Internal()->region_id = value;
}


int Combination::Aircode(void) const throw() {
    return Internal()->aircode;
}


void Combination::SetAircode(int value) throw() {
    Internal()->aircode = value;
}


int Combination::GiftMaterial(void) const throw() {
    return Internal()->GIFTmater;
}


void Combination::SetGiftMaterial(int value) throw() {
    Internal()->GIFTmater = value;
}


int Combination::LineOfSight(void) const throw() {
    return Internal()->los;
}


void Combination::SetLineOfSight(int value) throw() {
    Internal()->los = value;
}


bool Combination::HasColor(void) const throw() {
    return Internal()->rgb_valid != 0;
}


void Combination::SetHasColor(bool value) throw() {
    Internal()->rgb_valid = (value) ? '\1' : '\0';
}


unsigned char Combination::Red(void) const throw() {
    return Internal()->rgb[0];
}


void Combination::SetRed(unsigned char value) throw() {
    Internal()->rgb[0] = value;
}


unsigned char Combination::Green(void) const throw() {
    return Internal()->rgb[1];
}


void Combination::SetGreen(unsigned char value) throw() {
    Internal()->rgb[1] = value;
}


unsigned char Combination::Blue(void) const throw() {
    return Internal()->rgb[2];
}


void Combination::SetBlue(unsigned char value) throw() {
    Internal()->rgb[2] = value;
}


const char* Combination::Shader(void) const throw() {
    return bu_vls_addr(&Internal()->shader);
}


void Combination::SetShader(const char* value) throw() {
    bu_vls_strcpy(&Internal()->shader, value);
}


bool Combination::Inherit(void) const throw() {
    return Internal()->inherit != 0;
}


void Combination::SetInherit(bool value) throw() {
    Internal()->inherit = (value) ? '\1' : '\0';
}


const char* Combination::Material(void) const throw() {
    return bu_vls_addr(&Internal()->material);
}


void Combination::SetMaterial(const char* value) throw() {
    bu_vls_strcpy(&Internal()->material, value);
}


double Combination::Temperature(void) const throw() {
    return Internal()->temperature;
}


void Combination::SetTemperature(double value) throw() {
    Internal()->temperature = static_cast<float>(value);
}


const Object& Combination::operator=
(
    const Object& original
) throw() {
    const Combination* comb = dynamic_cast<const Combination*>(&original);
    assert(comb != 0);

    if (comb != 0)
        *this = *comb;

    return *this;
}


Object* Combination::Clone(void) const throw(std::bad_alloc) {
    return new Combination(*this);
}


const char* Combination::ClassName(void) throw() {
    return "Combination";
}


const char* Combination::Type(void) const throw() {
    return ClassName();
}


bool Combination::IsValid(void) const throw() {
    bool ret = Validate();

    if (ret) {
        if (!BU_SETJUMP) {
            const rt_comb_internal* internalp = Internal();

            // check operators in tree
            db_ck_tree(internalp->tree);

            if (internalp->region_flag == 0)
                ret = (db_is_tree_all_unions(internalp->tree) == 1);
        }
        else
            ret = false;

        BU_UNSETJUMP;
    }

    return ret;
}


Combination::Combination
(
    resource*       resp,
    directory*      pDir,
    rt_db_internal* ip,
    db_i*           dbip
) throw() : Object(resp, pDir, ip, dbip), m_internalp(0) {}


const rt_comb_internal* Combination::Internal(void) const {
    const rt_comb_internal* ret;

    if (m_ip != 0)
        ret = static_cast<const rt_comb_internal*>(m_ip->idb_ptr);
    else
        ret = m_internalp;

    RT_CHECK_COMB(ret);

    return ret;
}


rt_comb_internal* Combination::Internal(void) {
    rt_comb_internal* ret;

    if (m_ip != 0)
        ret = static_cast<rt_comb_internal*>(m_ip->idb_ptr);
    else
        ret = m_internalp;

    RT_CHECK_COMB(ret);

    return ret;
}
