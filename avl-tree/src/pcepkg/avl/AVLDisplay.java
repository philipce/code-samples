package pcepkg.avl;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.JRadioButton;
import javax.swing.JCheckBox;
import javax.swing.JButton;
import javax.swing.JScrollPane;
import javax.swing.ButtonGroup;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Dimension;
import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;

/** 
 * A GUI that displays the contents of an AVL tree and allows the user to modify the tree.
 *
 * @author  Philip Erickson
 * @version 1.0
 */
@SuppressWarnings("serial") 
public class AVLDisplay extends JFrame 
{
    //-------------------------------------------    
    // M E M B E R   D A T A 
    //-------------------------------------------
    
    private static final Color PERIMETER_COLOR = new Color(51,153,102);
    private static final Color WINDOW_COLOR = new Color(204, 255, 153);

    private AVLTree<String> model;
    private JPanel mainPanel;
    private JPanel treePanel;
    private WordPanel wordPanel;
    private JScrollPane scrollpane;


    //-------------------------------------------    
    // C O N S T R U C T O R S
    //-------------------------------------------

    /**
     * AVL tree display constructor.
     */
    public AVLDisplay() 
    {
	    super("AVL Display - type and press <Enter> to modify");
	    model = new AVLTree<String>();
	    mainPanel = new MainPanel();
	    setContentPane(mainPanel);
	    setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	    pack();
	    setVisible(true);
    }


    //-------------------------------------------    
    // P U B L I C   M E T H O D S
    //-------------------------------------------
    
    /**
     * Update the display to reflect actual tree contents.
     */      
    private void updateDisplay() 
    {
        setTitle("AVL Display - tree contains " + model.size() + " words");
	    wordPanel.update(); 	    
	    treePanel.revalidate();
	    treePanel.repaint();
    }


    //-------------------------------------------    
    // N E S T E D   C L A S S E S
    //-------------------------------------------

    /**
     * Main panel for GUI. Displays the tree and the interface for modifying the tree.
     */    
    private class MainPanel extends JPanel 
    {
        /**
         * Main panel constructor.
         */    
	    public MainPanel() 
	    {
	        wordPanel = new WordPanel();
	        treePanel = new TreePanel();
	        scrollpane = new JScrollPane(treePanel);
	    
	        setLayout(new BorderLayout());       	        
	        add(wordPanel, BorderLayout.NORTH);
	        add(scrollpane, BorderLayout.CENTER);
	    }
    }

    /**
     * Word panel for GUI. Displays the interface for modifying the tree.
     */   
    private class WordPanel extends JPanel 
    {
	    private JLabel wordLabel;
	    private JTextField wordField;
	    private JRadioButton addRB;
	    private JRadioButton removeRB;
	    private ButtonGroup rbGroup;
	    private JButton clearButton;

        /**
         * Word panel constructor.
         */   
        public WordPanel() 
	    {
	        setBackground(PERIMETER_COLOR);

	        // set up field for user to type word and take action when enter is pressed
	        wordLabel = new JLabel("Word: ");
	        final int FIELD_SIZE = 10;
	        wordField = new JTextField(FIELD_SIZE);
	        wordField.addActionListener(new ActionListener() {
                public void actionPerformed( ActionEvent e ) {
	                String w = wordField.getText();
	                if (!w.isEmpty() && addRB.isSelected())
	                    model.add(w);
                    else if (!w.isEmpty() && removeRB.isSelected())
                        model.remove(w);
                    updateDisplay();
                } });
	                
            // set up radio button to add word and focus on text field when clicked
	        addRB = new JRadioButton("Add Word");
	        addRB.setBackground(PERIMETER_COLOR);
	        addRB.addActionListener(new ActionListener() {
                public void actionPerformed( ActionEvent e ) {
		            wordField.requestFocus(); 
	            } });

            // set up radio button to remove word and focus on text field when clicked
	        removeRB = new JRadioButton("Remove Word");
	        removeRB.setBackground(PERIMETER_COLOR);
	        removeRB.addActionListener(new ActionListener() {
                public void actionPerformed( ActionEvent e ) {
		            wordField.requestFocus(); 
	            } });

            // ensure that only one radio button can be selected and default to add
	        rbGroup = new ButtonGroup();
	        rbGroup.add(addRB);
	        rbGroup.add(removeRB);
	        addRB.setSelected(true);

            // set up button to clear tree set alt-C keyboard shortcut, initially disabled
	        clearButton = new JButton("Clear");
	        clearButton.setMnemonic(KeyEvent.VK_C);
	        clearButton.setEnabled(false);
	        clearButton.addActionListener(new ActionListener() {
	            public void actionPerformed( ActionEvent e ) {
		            model.clear();
		            updateDisplay();
	            } });

            // add components
	        add(wordLabel);
	        add(wordField);
	        add(addRB);
	        add(removeRB);
	        add(clearButton);
	    }
	    
	    /**
	     * Update the word panel
	     */
	    void update()
	    {
	        wordField.setText("");
	        wordField.requestFocus();
	        clearButton.setEnabled(model.size() > 0);
	    }
    }    

    /**
     * Tree panel for GUI. Displays the contents of the tree graphically.
     */   
    private class TreePanel extends JPanel 
    {   	    
	    private static final int MIN_HEIGHT = 8 * AVLTree.ROW_HEIGHT;
	    private static final int MARGIN = 20;
	    
        /**
         * Tree panel constructor.
         */
        public TreePanel() 
	    {
	        super();
	        setBackground(WINDOW_COLOR);
	    }

        /*
         * Return preferred window size. Used for scroll pane.
         */
        @Override
        public Dimension getPreferredSize() 
	    {
	        int h = Math.max(MIN_HEIGHT, model.drawHeight());
	        int w = model.drawWidth(getFontMetrics(getFont())); 
	        
	        return new Dimension(w + MARGIN * 2, h + MARGIN * 2);
	    }

        /*
         * Draw tree panel.
         */
        @Override     
	    public void paintComponent(Graphics g) 
	    {
	        super.paintComponent(g);
	        int w = getSize().width;
            int h = getSize().height;	        
	        model.draw(g, w, h, getFontMetrics(getFont()));
	    }		    
    }
}

