/* Ordered {set,map} data type implemented by a binary tree.
   Copyright (C) 2006-2007, 2009-2021 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* An AVL tree is a binary tree where
   1. The height of each node is calculated as
        heightof(node) = 1 + max (heightof(node.left), heightof(node.right)).
   2. The heights of the subtrees of each node differ by at most 1:
        | heightof(right) - heightof(left) | <= 1.
   3. The index of the elements in the node.left subtree are smaller than
      the index of node.
      The index of the elements in the node.right subtree are larger than
      the index of node.
 */

/* Tree node implementation, valid for this file only.  */
struct NODE_IMPL
{
  struct NODE_IMPL *left;   /* left branch, or NULL */
  struct NODE_IMPL *right;  /* right branch, or NULL */
  /* Parent pointer, or NULL. The parent pointer is not needed for most
     operations.  It is needed so that a NODE_T can be returned without
     memory allocation, on which the functions <container>_remove_node,
     <container>_add_before, <container>_add_after can be implemented.  */
  struct NODE_IMPL *parent;
  int balance;                      /* heightof(right) - heightof(left),
                                       always = -1 or 0 or 1 */
  NODE_PAYLOAD_FIELDS
};
typedef struct NODE_IMPL * NODE_T;

/* Concrete CONTAINER_IMPL type, valid for this file only.  */
struct CONTAINER_IMPL
{
  struct CONTAINER_IMPL_BASE base;
  struct NODE_IMPL *root;           /* root node or NULL */
  size_t count;                     /* number of nodes */
};

/* An AVL tree of height h has at least F_(h+2) - 1 [Fibonacci number] and at
   most 2^h - 1 elements.  So, h <= 84 (because a tree of height h >= 85 would
   have at least F_87 - 1 elements, and because even on 64-bit machines,
     sizeof (NODE_IMPL) * (F_87 - 1) > 2^64
   this would exceed the address space of the machine.  */
#define MAXHEIGHT 83

/* Ensures the tree is balanced, after an insertion or deletion operation.
   The height of NODE is incremented by HEIGHT_DIFF (1 or -1).
   PARENT = NODE->parent.  (NODE can also be NULL.  But PARENT is non-NULL.)
   Rotation operations are performed starting at PARENT (not NODE itself!).  */
static void
rebalance (CONTAINER_T container,
           NODE_T node, int height_diff, NODE_T parent)
{
  for (;;)
    {
      NODE_T child;
      int previous_balance;
      int balance_diff;
      NODE_T nodeleft;
      NODE_T noderight;

      child = node;
      node = parent;

      previous_balance = node->balance;

      /* The balance of NODE is incremented by BALANCE_DIFF: +1 if the right
         branch's height has increased by 1 or the left branch's height has
         decreased by 1, -1 if the right branch's height has decreased by 1 or
         the left branch's height has increased by 1, 0 if no height change.  */
      if (node->left != NULL || node->right != NULL)
        balance_diff = (child == node->right ? height_diff : -height_diff);
      else
        /* Special case where above formula doesn't work, because the caller
           didn't tell whether node's left or right branch shrunk from height 1
           to NULL.  */
        balance_diff = - previous_balance;

      node->balance += balance_diff;
      if (balance_diff == previous_balance)
        {
          /* node->balance is outside the range [-1,1].  Must rotate.  */
          NODE_T *nodep;

          if (node->parent == NULL)
            /* node == container->root */
            nodep = &container->root;
          else if (node->parent->left == node)
            nodep = &node->parent->left;
          else if (node->parent->right == node)
            nodep = &node->parent->right;
          else
            abort ();

          nodeleft = node->left;
          noderight = node->right;

          if (balance_diff < 0)
            {
              /* node->balance = -2.  The subtree is heavier on the left side.
                 Rotate from left to right:

                                  *
                                /   \
                             h+2      h
               */
              NODE_T nodeleftright = nodeleft->right;
              if (nodeleft->balance <= 0)
                {
                  /*
                              *                    h+2|h+3
                            /   \                  /    \
                         h+2      h      -->      /   h+1|h+2
                         / \                      |    /    \
                       h+1 h|h+1                 h+1  h|h+1  h
                   */
                  node->left = nodeleftright;
                  nodeleft->right = node;

                  nodeleft->parent = node->parent;
                  node->parent = nodeleft;
                  if (nodeleftright != NULL)
                    nodeleftright->parent = node;

                  nodeleft->balance += 1;
                  node->balance = - nodeleft->balance;

                  *nodep = nodeleft;
                  height_diff = (height_diff < 0
                                 ? /* noderight's height had been decremented from
                                      h+1 to h.  The subtree's height changes from
                                      h+3 to h+2|h+3.  */
                                   nodeleft->balance - 1
                                 : /* nodeleft's height had been incremented from
                                      h+1 to h+2.  The subtree's height changes from
                                      h+2 to h+2|h+3.  */
                                   nodeleft->balance);
                }
              else
                {
                  /*
                            *                     h+2
                          /   \                 /     \
                       h+2      h      -->    h+1     h+1
                       / \                    / \     / \
                      h  h+1                 h   L   R   h
                         / \
                        L   R

                   */
                  NODE_T L = nodeleft->right = nodeleftright->left;
                  NODE_T R = node->left = nodeleftright->right;
                  nodeleftright->left = nodeleft;
                  nodeleftright->right = node;

                  nodeleftright->parent = node->parent;
                  if (L != NULL)
                    L->parent = nodeleft;
                  if (R != NULL)
                    R->parent = node;
                  nodeleft->parent = nodeleftright;
                  node->parent = nodeleftright;

                  nodeleft->balance = (nodeleftright->balance > 0 ? -1 : 0);
                  node->balance = (nodeleftright->balance < 0 ? 1 : 0);
                  nodeleftright->balance = 0;

                  *nodep = nodeleftright;
                  height_diff = (height_diff < 0
                                 ? /* noderight's height had been decremented from
                                      h+1 to h.  The subtree's height changes from
                                      h+3 to h+2.  */
                                   -1
                                 : /* nodeleft's height had been incremented from
                                      h+1 to h+2.  The subtree's height changes from
                                      h+2 to h+2.  */
                                   0);
                }
            }
          else
            {
              /* node->balance = 2.  The subtree is heavier on the right side.
                 Rotate from right to left:

                                  *
                                /   \
                              h      h+2
               */
              NODE_T noderightleft = noderight->left;
              if (noderight->balance >= 0)
                {
                  /*
                              *                    h+2|h+3
                            /   \                   /    \
                          h      h+2     -->    h+1|h+2   \
                                 / \            /    \    |
                             h|h+1 h+1         h   h|h+1 h+1
                   */
                  node->right = noderightleft;
                  noderight->left = node;

                  noderight->parent = node->parent;
                  node->parent = noderight;
                  if (noderightleft != NULL)
                    noderightleft->parent = node;

                  noderight->balance -= 1;
                  node->balance = - noderight->balance;

                  *nodep = noderight;
                  height_diff = (height_diff < 0
                                 ? /* nodeleft's height had been decremented from
                                      h+1 to h.  The subtree's height changes from
                                      h+3 to h+2|h+3.  */
                                   - noderight->balance - 1
                                 : /* noderight's height had been incremented from
                                      h+1 to h+2.  The subtree's height changes from
                                      h+2 to h+2|h+3.  */
                                   - noderight->balance);
                }
              else
                {
                  /*
                            *                    h+2
                          /   \                /     \
                        h      h+2    -->    h+1     h+1
                               / \           / \     / \
                             h+1  h         h   L   R   h
                             / \
                            L   R

                   */
                  NODE_T L = node->right = noderightleft->left;
                  NODE_T R = noderight->left = noderightleft->right;
                  noderightleft->left = node;
                  noderightleft->right = noderight;

                  noderightleft->parent = node->parent;
                  if (L != NULL)
                    L->parent = node;
                  if (R != NULL)
                    R->parent = noderight;
                  node->parent = noderightleft;
                  noderight->parent = noderightleft;

                  node->balance = (noderightleft->balance > 0 ? -1 : 0);
                  noderight->balance = (noderightleft->balance < 0 ? 1 : 0);
                  noderightleft->balance = 0;

                  *nodep = noderightleft;
                  height_diff = (height_diff < 0
                                 ? /* nodeleft's height had been decremented from
                                      h+1 to h.  The subtree's height changes from
                                      h+3 to h+2.  */
                                   -1
                                 : /* noderight's height had been incremented from
                                      h+1 to h+2.  The subtree's height changes from
                                      h+2 to h+2.  */
                                   0);
                }
            }
          node = *nodep;
        }
      else
        {
          /* No rotation needed.  Only propagation of the height change to the
             next higher level.  */
          if (height_diff < 0)
            height_diff = (previous_balance == 0 ? 0 : -1);
          else
            height_diff = (node->balance == 0 ? 0 : 1);
        }

      if (height_diff == 0)
        break;

      parent = node->parent;
      if (parent == NULL)
        break;
    }
}

static NODE_T
gl_tree_nx_add_first (CONTAINER_T container, NODE_PAYLOAD_PARAMS)
{
  /* Create new node.  */
  NODE_T new_node =
    (struct NODE_IMPL *) malloc (sizeof (struct NODE_IMPL));

  if (new_node == NULL)
    return NULL;

  new_node->left = NULL;
  new_node->right = NULL;
  new_node->balance = 0;
  NODE_PAYLOAD_ASSIGN(new_node)

  /* Add it to the tree.  */
  if (container->root == NULL)
    {
      container->root = new_node;
      new_node->parent = NULL;
    }
  else
    {
      NODE_T node;

      for (node = container->root; node->left != NULL; )
        node = node->left;

      node->left = new_node;
      new_node->parent = node;
      node->balance--;

      /* Rebalance.  */
      if (node->right == NULL && node->parent != NULL)
        rebalance (container, node, 1, node->parent);
    }

  container->count++;
  return new_node;
}

/* Adds the already allocated NEW_NODE to the tree, right before NODE.  */
static void
gl_tree_add_node_before (CONTAINER_T container, NODE_T node, NODE_T new_node)
{
  bool height_inc;

  new_node->left = NULL;
  new_node->right = NULL;
  new_node->balance = 0;

  /* Add it to the tree.  */
  if (node->left == NULL)
    {
      node->left = new_node;
      node->balance--;
      height_inc = (node->right == NULL);
    }
  else
    {
      for (node = node->left; node->right != NULL; )
        node = node->right;
      node->right = new_node;
      node->balance++;
      height_inc = (node->left == NULL);
    }
  new_node->parent = node;

  /* Rebalance.  */
  if (height_inc && node->parent != NULL)
    rebalance (container, node, 1, node->parent);

  container->count++;
}

static NODE_T
gl_tree_nx_add_before (CONTAINER_T container, NODE_T node, NODE_PAYLOAD_PARAMS)
{
  /* Create new node.  */
  NODE_T new_node =
    (struct NODE_IMPL *) malloc (sizeof (struct NODE_IMPL));

  if (new_node == NULL)
    return NULL;

  NODE_PAYLOAD_ASSIGN(new_node)

  gl_tree_add_node_before (container, node, new_node);
  return new_node;
}

/* Adds the already allocated NEW_NODE to the tree, right after NODE.  */
static void
gl_tree_add_node_after (CONTAINER_T container, NODE_T node, NODE_T new_node)
{
  bool height_inc;

  new_node->left = NULL;
  new_node->right = NULL;
  new_node->balance = 0;

  /* Add it to the tree.  */
  if (node->right == NULL)
    {
      node->right = new_node;
      node->balance++;
      height_inc = (node->left == NULL);
    }
  else
    {
      for (node = node->right; node->left != NULL; )
        node = node->left;
      node->left = new_node;
      node->balance--;
      height_inc = (node->right == NULL);
    }
  new_node->parent = node;

  /* Rebalance.  */
  if (height_inc && node->parent != NULL)
    rebalance (container, node, 1, node->parent);

  container->count++;
}

static NODE_T
gl_tree_nx_add_after (CONTAINER_T container, NODE_T node, NODE_PAYLOAD_PARAMS)
{
  /* Create new node.  */
  NODE_T new_node =
    (struct NODE_IMPL *) malloc (sizeof (struct NODE_IMPL));

  if (new_node == NULL)
    return NULL;

  NODE_PAYLOAD_ASSIGN(new_node)

  gl_tree_add_node_after (container, node, new_node);
  return new_node;
}

static void
gl_tree_remove_node_no_free (CONTAINER_T container, NODE_T node)
{
  NODE_T parent = node->parent;

  if (node->left == NULL)
    {
      /* Replace node with node->right.  */
      NODE_T child = node->right;

      if (child != NULL)
        child->parent = parent;
      if (parent == NULL)
        container->root = child;
      else
        {
          if (parent->left == node)
            parent->left = child;
          else /* parent->right == node */
            parent->right = child;

          rebalance (container, child, -1, parent);
        }
    }
  else if (node->right == NULL)
    {
      /* It is not absolutely necessary to treat this case.  But the more
         general case below is more complicated, hence slower.  */
      /* Replace node with node->left.  */
      NODE_T child = node->left;

      child->parent = parent;
      if (parent == NULL)
        container->root = child;
      else
        {
          if (parent->left == node)
            parent->left = child;
          else /* parent->right == node */
            parent->right = child;

          rebalance (container, child, -1, parent);
        }
    }
  else
    {
      /* Replace node with the rightmost element of the node->left subtree.  */
      NODE_T subst;
      NODE_T subst_parent;
      NODE_T child;

      for (subst = node->left; subst->right != NULL; )
        subst = subst->right;

      subst_parent = subst->parent;

      child = subst->left;

      /* The case subst_parent == node is special:  If we do nothing special,
         we get confusion about node->left, subst->left and child->parent.
           subst_parent == node
           <==> The 'for' loop above terminated immediately.
           <==> subst == subst_parent->left
                [otherwise subst == subst_parent->right]
         In this case, we would need to first set
           child->parent = node; node->left = child;
         and later - when we copy subst into node's position - again
           child->parent = subst; subst->left = child;
         Altogether a no-op.  */
      if (subst_parent != node)
        {
          if (child != NULL)
            child->parent = subst_parent;
          subst_parent->right = child;
        }

      /* Copy subst into node's position.
         (This is safer than to copy subst's value into node, keep node in
         place, and free subst.)  */
      if (subst_parent != node)
        {
          subst->left = node->left;
          subst->left->parent = subst;
        }
      subst->right = node->right;
      subst->right->parent = subst;
      subst->balance = node->balance;
      subst->parent = parent;
      if (parent == NULL)
        container->root = subst;
      else if (parent->left == node)
        parent->left = subst;
      else /* parent->right == node */
        parent->right = subst;

      /* Rebalancing starts at child's parent, that is subst_parent -
         except when subst_parent == node.  In this case, we need to use
         its replacement, subst.  */
      rebalance (container, child, -1, subst_parent != node ? subst_parent : subst);
    }

  container->count--;
}

static bool
gl_tree_remove_node (CONTAINER_T container, NODE_T node)
{
  gl_tree_remove_node_no_free (container, node);
  NODE_PAYLOAD_DISPOSE (container, node)
  free (node);
  return true;
}

/* For debugging.  */
static unsigned int
check_invariants (NODE_T node, NODE_T parent, size_t *counterp)
{
  unsigned int left_height =
    (node->left != NULL ? check_invariants (node->left, node, counterp) : 0);
  unsigned int right_height =
    (node->right != NULL ? check_invariants (node->right, node, counterp) : 0);
  int balance = (int)right_height - (int)left_height;

  if (!(node->parent == parent))
    abort ();
  if (!(balance >= -1 && balance <= 1))
    abort ();
  if (!(node->balance == balance))
    abort ();

  (*counterp)++;

  return 1 + (left_height > right_height ? left_height : right_height);
}
