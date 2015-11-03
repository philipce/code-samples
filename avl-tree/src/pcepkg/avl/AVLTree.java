package pcepkg.avl;

import java.util.Collection;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.Iterator;
import java.lang.Comparable;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.FontMetrics;

/** 
 * A binary search tree that contains no duplicate elements and maintains even balance by rotating
 * tree nodes as the tree changes size.
 *
 * @author  Philip Erickson
 * @version 1.0
 */
public class AVLTree<E extends Comparable<E>> implements Set<E>
{
    //-------------------------------------------    
    // M E M B E R   D A T A 
    //-------------------------------------------
         
    // Parameters for drawing tree  
    public static final int ROW_HEIGHT = 40; 
    public static final int WORD_SPACE = 4; 
    public static final int LINE_SPACE = 2;
    public static final int BLANK_HEIGHT = 8;
    public static final int WORD_HEIGHT = 4;
    private int xroot;
     
    private Node root;
    private int size;   


    //-------------------------------------------    
    // C O N S T R U C T O R S
    //-------------------------------------------

    /**
     * AVL tree constructor.
     */
    public AVLTree ()
    {
        root = null;
        size = 0;
    }


    //-------------------------------------------    
    // P U B L I C   M E T H O D S
    //-------------------------------------------

    /**
     * Add the given element to the tree if not already present.
     * 
     * @param e the element to add
     * @return true if the element was added to the tree
     */      
    @Override   
    public boolean add(E e)
    {
        if (this.contains(e))
            return false;

        root = insert(root, e);
        size++;

        return true;
    }    
    
    /**
     * Removes all elements from the tree, leaving it empty.
     */
    @Override 
    public void clear()
    {
        root = null;
        size = 0;
        
        return;
    }

    /**
     * Determines whether or not the tree contains the given element.
     * 
     * @param o the element to check 
     * @return true if the element is contained in the tree
     */          
    @Override 
    @SuppressWarnings("unchecked")
    public boolean contains(Object o)
    {
        if (!(o instanceof Comparable))
            return false;
        
        return find(root, (E) o);
    }

    /**
     * Determines whether or not the tree is empty.
     * 
     * @return true if the element is contained in the tree
     */    
    @Override  
    public boolean isEmpty()
    {  
        return root == null;
    }
    
    /**
     * Return an iterator over the elements in the tree.
     * 
     * @return an iterator over the tree
     */ 
    @Override     
    public Iterator<E> iterator()
    {
        return new AVLIterator<E>(root);
    }

    /**
     * Remove the given element from the tree.
     * 
     * @param o the element to remove 
     * @return true if the element was removed from the tree
     */  
    @Override    
    @SuppressWarnings("unchecked")
    public boolean remove(Object o)
    {
        if (!this.contains(o))
            return false;

        root = delete(root, (E) o);
        size--;

        return true;
    }

    /**
     * Return the number of elements in the tree.
     * 
     * @return the number of elements in the tree
     */  
    @Override    
    public int size()
    {
        return size;
    }
    
    /**
     * Return the height of the tree, in terms of nodes.
     *
     * @return height of the tree
     */
    public int height() 
    {        
        if (root == null)
            return 0;
        else
            return root.getHeight();
    }

    /**
     * Return the root node of the tree.
     * 
     * @return the root node of the tree
     */     
    public Node getRootNode()
    {
        return root;
    }
        
    /**
     * Return an array containing all the elements in the tree, in the order encountered by 
     * preorder traversal.
     * 
     * @return array of elements in the tree
     */     
    @Override 
    public Object[] toArray()
    {
        List<Object> list = new ArrayList<Object>();
        traversePreorder(root, list);

        return list.toArray();
    }
    
    /**
     * Draw this tree on a given graphics object.
     * 
     * @param g graphics object on which to draw tree
     * @param panelWidth width of panel on which tree will be displayed
     * @param panelHeight height of panel on which tree will be displayed
     * @param fm specifies the display font, to determine width of node's string value
     * @return width of the tree
     */
    public int draw(Graphics g, int panelWidth, int panelHeight, FontMetrics fm) 
    {
        int x = (panelWidth - drawWidth(fm)) / 2;
        int y = (panelHeight - drawHeight()) / 2;
    
        return draw(g, root, x, y, fm);
    } 
    
    /**
     * Return the width of the drawn image of this tree
     *
     * @return drawn tree width
     */
    public int drawWidth(FontMetrics fm)
    {
        return width(root, fm);
    }      
    
    /**
     * Return the height of the drawn image of this tree
     *
     * @return drawn tree height
     */
    public int drawHeight()
    {
        return height() * ROW_HEIGHT;
    }
         
    // Unsupported Operations
    @Override
    public boolean addAll(Collection<? extends E> c){ throw new UnsupportedOperationException(); }
    
    @Override
    public boolean containsAll(Collection<?> c){ throw new UnsupportedOperationException(); }
    
    @Override
    public boolean removeAll(Collection<?> c){ throw new UnsupportedOperationException(); }
    
    @Override
    public boolean retainAll(Collection<?> c){ throw new UnsupportedOperationException(); }
    
    @Override
    public <T> T[] toArray(T[] a){ throw new UnsupportedOperationException(); }


    //-------------------------------------------    
    // P R I V A T E   M E T H O D S
    //-------------------------------------------
        
    /**
     * Performs a single left rotation on a given node.
     * 
     * @param k1 the node to be rotated
     * @return the rotated node      
     */
    private Node rotateSL(Node k1)
    {        
        Node k2 = k1.getRightChild();
        k1.setRightChild(k2.getLeftChild());
        k2.setLeftChild(k1);
        k1.updateHeight();
        k2.updateHeight();   

        return k2;
    }
    
    /**
     * Performs a single right rotation on a given node.
     * 
     * @param k2 the node to be rotated
     * @return the rotated node      
     */
    private Node rotateSR(Node k2)
    {
        Node k1 = k2.getLeftChild();

        k2.setLeftChild(k1.getRightChild());
        k1.setRightChild(k2);
        k2.updateHeight();
        k1.updateHeight();

        return k1;
    }

    /**
     * Performs a double left rotation on a given node.
     * 
     * @param k1 the node to be rotated
     * @return the rotated node      
     */
    private Node rotateDL(Node k1)
    {
        k1.setRightChild(rotateSR(k1.getRightChild()));
        
        return rotateSL(k1);
    }

    /**
     * Performs a double right rotation on a given node.
     * 
     * @param k3 the node to be rotated
     * @return the rotated node      
     */
    private Node rotateDR(Node k3)
    {
        k3.setLeftChild(rotateSL(k3.getLeftChild()));
        return rotateSR(k3);
    }

    /**
     * Determines how the given node ought to be rotated based on height difference.
     * 
     * @param n the node to determine rotation for
     * @param difference height difference between children  
     * @return the rotation for the node      
     */
    private Rotation determineRotation(Node n, int difference)
    {
        Rotation r = Rotation.NONE;

        // if tree is left heavy, rotate to the right
        if (difference > 0)
        {   
            // if tree's left child is left heavy, do single right
            if (n.getLeftChild().getLeftHeight() >= n.getLeftChild().getRightHeight())
            {
               r = Rotation.SINGLE_RIGHT;
            }
            // if tree's left child is right heavy, do double right
            else
            {
               r = Rotation.DOUBLE_RIGHT;
            }
        }        
        // if tree is right heavy, rotate to the left
        else if (difference < 0)
        {
            // if tree's right child is right heavy, do single left
            if (n.getRightChild().getRightHeight() >= n.getRightChild().getLeftHeight())
            {
               r = Rotation.SINGLE_LEFT;
            }          
            // if tree's right child is left heavy, do double left  
            else
            {
               r = Rotation.DOUBLE_LEFT;
            }
        }

        return r;
    }

    /**
     * Balance the given node.
     * 
     * @param n the node to balance
     * @return the balanced node      
     */
    private Node balance(Node n)
    {
        Rotation type;
        Node balancedNode = n;
            
        // node requires balancing if balance factor is more than 1
        int left = n.getLeftHeight();
        int right = n.getRightHeight();
        int difference = left-right;
        if (Math.abs(difference) > 1)
        {
            type = determineRotation(n, difference);
            switch (type)
            {
                case SINGLE_RIGHT:
                    balancedNode = rotateSR(n);
                    break;
                case SINGLE_LEFT:
                    balancedNode = rotateSL(n);
                    break;
                case DOUBLE_RIGHT:
                    balancedNode = rotateDR(n);
                    break;
                case DOUBLE_LEFT:
                    balancedNode = rotateDL(n);
                    break;
                case NONE:
                    break;
                default:
                    throw new RuntimeException("Invalid rotation type");
            }
        }
        
        return balancedNode;
    }
    
    /**
     * Insert new data into tree.
     * 
     * Compares new data to the root and recursively inserts into the left or right child of the
     * root node, based on the compareTo() comparison. Recursion continues until the root node
     * doesn't have a child on the side to be inserted, at which point the new data node is created.
     * After insertion, the root node height is updated and the tree rebalanced.
     *
     * @param n root node of the tree to insert into
     * @param newData data to insert     
     * @return root node of the tree after insertion     
     */
    private Node insert(Node n, E newData)
    {     
        // base case: if tree is empty, the data being inserted becomes root
        if (n == null)
        {
            n = new Node(newData);
            return n;
        }

        // recurse: insert new node on root's left
        if (newData.compareTo(n.getData()) < 0)  
        {
            Node leftSubtree = insert(n.getLeftChild(), newData);
            n.setLeftChild(leftSubtree);
        }
        // recurse: insert new node on root's right
        else if (newData.compareTo(n.getData()) > 0) 
        {
            Node rightSubtree = insert(n.getRightChild(), newData);
            n.setRightChild(rightSubtree);
        }

        // update and balance tree 
        n.updateHeight();
        n = balance(n);

        return n;
    }

    /**
     * Delete a given item from the tree.
     * 
     * @param n root node of tree
     * @param item item to remove from tree    
     * @return root node of the tree after deletion     
     */
    private Node delete(Node n, E item)
    {
        // base case: nothing left to search for removal
        if (n == null)
            return n;          

        // recurse: search left subtree for item to remove
        if (item.compareTo(n.getData()) < 0)
        {
            Node leftSubtree = delete(n.getLeftChild(), item);
            n.setLeftChild(leftSubtree);
        }
        // recurse: search right subtree for item to remove
        else if (item.compareTo(n.getData()) > 0)
        {
            Node rightSubtree = delete(n.getRightChild(), item);
            n.setRightChild(rightSubtree);
        }
        // recurse: remove current node
        else
        {
            //case 1: remove leaf node
            if (n.getRightChild() == null && n.getLeftChild() == null)
            {
                return null;
            }
            //case 2: remove node with left child only
            else if (n.getRightChild() == null && n.getLeftChild() != null)
            {  
                return n.getLeftChild();
            }
            //case 3: remove node with right child only
            else if (n.getRightChild() != null && n.getLeftChild() == null)
            {
                return n.getRightChild();
            }
            //case 4: delete node with 2 children (recursive)
            else
            {
                // find smallest element in right subtree 
                Node leastNode = findLeastRH(n);
                assert leastNode != null; // case 4 implies right subtree must have some element               
                
                // overwrite current node data with smallest element
                n.setData(leastNode.getData());
                
                // remove duplicate least node from right subtree
                n.setRightChild(delete(n.getRightChild(), leastNode.getData()));
            }
        }
          
        // update and balance tree  
        n.updateHeight();
        n = balance(n);

        return n;
    }

    /**
     * Find the least element of the right-hand side of a given tree.
     * 
     * @param n root node of tree  
     * @return least element, or null if right subtree is empty     
     */    
    private Node findLeastRH(Node n)
    {
        // handle empty subtree
        if (n.getRightChild() == null)
            return null;

        // begin at right child
        n = n.getRightChild();
        
        // continue left, getting smaller, until nothing else is left
        while (n.getLeftChild() != null)
            n = n.getLeftChild();

        return n;
    }
    
    /**
     * Find if a given tree contains an item.
     * 
     * Method recursively searches tree for the given item.
     *
     * @param n root node of tree  
     * @param item item to find within tree     
     * @return true if tree contains item, false otherwise
     */  
    private boolean find(Node n, E item)
    {
        // base case: tree does not contain item  
        if(n == null)  
            return false;
        
        // extract current node data for comparison  
        E data = n.getData();

        // base case: current node is what we're looking for
        if (data.compareTo(item) == 0)
            return true;
        //recurse: search right subtree for item larger than current data     
        else if (data.compareTo(item) < 0)      
            return find (n.getRightChild(), item);

        //search left subtree for item smaller than current data
        else
            return find (n.getLeftChild(), item);
    }
    
    /**
     * Return the width string representation of the given subtree.
     *
     * @param t root node of the subtree
     * @param fm font metrics for determining string width
     * @return width of string representation of subtree
     */
    private int width(Node t, FontMetrics fm) 
    {
        // base case: empty tree has no width        
        if (t == null)
	        return 0;

        // get width of current node
        String word = (String) t.getData();
        int wordWidth = fm.stringWidth(word);
        int half = wordWidth / 2 + WORD_SPACE;

        // recursively find widest children
        int leftWidth = Math.max(half, width(t.getLeftChild(), fm));
        int rightWidth = Math.max(half, width(t.getRightChild(), fm));

        return leftWidth + rightWidth;
    }
    
    /**
     * Populate given list with elements, in the order given by preorder traversal of the tree.
     *
     * @param n root node of tree  
     * @param l the list to populate with tree elements   
     * @return true if tree contains item, false otherwise
     */      
    private void traversePreorder(Node n, List<Object> l)
    {
        if (n == null)
            return;
          
        l.add(n.getData());
        traversePreorder(n.getLeftChild(), l);
        traversePreorder(n.getRightChild(), l);

        return;
    }

    /**
     * Draw the current node to the given graphics object.
     *
     * @param g graphics object on which to draw tree
     * @param t node to draw
     * @param x horizontal location to draw at
     * @param y vertical location to draw at     
     * @param fm specifies the display font, to determine width of node's string value
     * @return width of the tree
     */
    private int draw(Graphics g, Node t, int x, int y, FontMetrics fm) 
    {
        // check for empty tree
        if (t == null)
	        return 0;

        // get the word to represent this node
        String word = (String) t.getData();
        int wordWidth = fm.stringWidth(word);
        int half = wordWidth/ 2;

        // draw the left child value
        int drawLeft = draw(g, t.getLeftChild(), x, y + ROW_HEIGHT, fm);
        int leftWidth = Math.max(half + WORD_SPACE, drawLeft);           
        int leftRoot = xroot;

        // draw the right child value
        int drawRight = draw(g, t.getRightChild(), x + leftWidth, y + ROW_HEIGHT, fm);
        int rightWidth = Math.max(half + WORD_SPACE, drawRight);
        int rightRoot = xroot;

        // draw this node value
        int width = leftWidth + rightWidth;
        xroot = x + width/2;
        g.drawString(word, xroot - half, y + WORD_HEIGHT);

        // draw edges connecting parent to children
        g.setColor(Color.black);
        if (t.getLeftChild() != null)
	        g.drawLine(xroot - LINE_SPACE, y + BLANK_HEIGHT, leftRoot + LINE_SPACE, 
	                   y + ROW_HEIGHT - BLANK_HEIGHT);
        if (t.getRightChild() != null)
	        g.drawLine(xroot + LINE_SPACE, y + BLANK_HEIGHT, rightRoot - LINE_SPACE, 
	                   y + ROW_HEIGHT - BLANK_HEIGHT);

        return width;
    }
        
    
    //-------------------------------------------    
    // N E S T E D   C L A S S E S
    //-------------------------------------------

    /**
     * Enumeration for the different types of rotations in an AVL tree balancing.
     */  
    private enum Rotation 
    {
        SINGLE_RIGHT, SINGLE_LEFT, DOUBLE_RIGHT, DOUBLE_LEFT, NONE;
    }    

    /**
     * Inner class for iterating over the elements of an AVL tree.
     */    
    private class AVLIterator<E> implements Iterator<E>
    {
        private Object[] array;
        private int index;
        
        /**
         * AVL tree iterator constructor.
         *
         * @param n root node of tree to iterate over
         */  
        public AVLIterator(Node n)
        {
            List<Object> list = new ArrayList<Object>();
            traversePreorder(n, list);
            array = list.toArray();
            index = 0;
        }

        /**
         * Determine if there is another element in the iteration.
         *
         * @return true if iteration has another element, false otherwise
         */  
        public boolean hasNext()
        {
            return index < array.length;
        }

        /**
         * Return the next element in the iteration.
         *
         * @return the next element
         */  
        @SuppressWarnings("unchecked")
        public E next()
        {            
            Object o = array[index++];
            
            return (E) o;
        }

        // Unsupported Operations
        public void remove(){ throw new UnsupportedOperationException(); }
    }

    /**
     * Nested static class representing a node in the AVL tree.
     */        
    private class Node
    {
        private E data;
        private Node leftChild;
        private Node rightChild;
        private int height;
        
        /**
         * AVL tree node constructor.
         *
         * @param newData data to insert into new node
         */  
        public Node(E newData)
        {
            data = newData;
            height = 0;
            leftChild = null;
            rightChild = null;
        }

        /**
         * Return the string representation of this node.
         *
         * @return this node as a string
         */  
        public String toString()
        {
            return (String) data;
        }
        
        /**
         * Update the height of this node.        
         */  
        public void updateHeight()
        {
            height = getTallestChildHeight() + 1;
        }
    
        /**
         * Return the height of this node.
         *
         * @return this node's height
         */  
        public int getHeight()
        {
            return height;
        }

        /**
         * Return the height of this node's left child, which is negative 1 if there is no child.
         *
         * @return left child's height
         */  
        public int getLeftHeight()
        {
            if (leftChild == null)
                return -1;
            else
                return leftChild.height;    
        }

        /**
         * Return the height of this node's right child, which is negative 1 if there is no child.
         *
         * @return right child's height
         */  
        public int getRightHeight()
        {
            if (rightChild == null)
                return -1;
            else
                return rightChild.height;    
        }

        /**
         * Return the data contained in this node.
         *
         * @return node data
         */  
        public E getData()
        {
            return data;
        }

        /**
         * Return the left child of this node.
         *
         * @return left child
         */  
        public Node getLeftChild()
        {
            return leftChild;
        }

        /**
         * Return the right child of this node.
         *
         * @return right child
         */  
        public Node getRightChild()
        {
            return rightChild;
        }

        /**
         * Set the left child of this node.
         *
         * @param n left child node
         */  
        public void setLeftChild(Node n)
        {
            leftChild = n;
        }
        
        /**
         * Set the right child of this node.
         *
         * @param n right child node
         */  
        public void setRightChild(Node n)
        {
            rightChild = n;
        }

        /**
         * Set the data contained in this node.
         *
         * @param newData data to set node to
         */  
        public void setData(E newData)
        {
            data = newData;
        }
        
        /**
         * Return the height of the tallest child of this node.
         *
         * @return the tallest child's height
         */  
        private int getTallestChildHeight()
        {
            int left = -1;
            int right = -1;

            if (leftChild != null)
               left = leftChild.height;

            if (rightChild != null)
               right = rightChild.height;

            return Math.max (left, right);                
        }
    } 
}
