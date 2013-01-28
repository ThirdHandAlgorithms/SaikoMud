using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using MySql.Data.MySqlClient;

namespace grideditor {
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window {
        protected int maxh, maxw;
        protected Info infowin;
        protected saikomudDataSetTableAdapters.gridTableAdapter adapter;
        protected saikomudDataSet.gridDataTable table;
        protected Point selectedroom;

        protected Point dragstart;
        protected Point dragcurrent;
        protected int iLinesDrawn;
        
        protected bool bDrag = false;
        //protected bool bMouseMove = false;

        public MainWindow() {
            InitializeComponent();

            infowin = new Info();
            infowin.Show();

            adapter = new saikomudDataSetTableAdapters.gridTableAdapter();
            table = new saikomudDataSet.gridDataTable();

            adapter.Fill(table);
        }

        // Create room coords at the position of the mouse pointer, stored in p
        private Point GetRoomPositionByMousePos( Point mouse ) {
            Point p = new Point();

            p.X = Math.Floor(mouse.X / 11.0);
            p.Y = Math.Floor(mouse.Y / 11.0);

            p.X -= Math.Floor(maxw / 2.0);
            p.Y = Math.Floor(maxh / 2.0) - p.Y;

            return p;
        }

        // Create a rectangle for a room, stored in r
        private Rect GetRectForRoom( Point room ) {
            int x = (int)(room.X + Math.Floor(maxw / 2.0)) * 11;
            int y = (int)(Math.Floor(maxh / 2.0) - room.Y) * 11;

            Rect r = new Rect(
                    x,
                    y,
                    x + 10,
                    y + 10
                );

            return r;
        }

        private bool isInDragSelection( Point pRoom )
        {
            bool bInX = false;
            bool bInY = false;

            if ( (dragcurrent.X >= dragstart.X) && (dragcurrent.Y >= dragstart.Y) )
            {
                bInX = (pRoom.X >= this.dragstart.X) && (pRoom.X <= this.dragcurrent.X);
                bInY = (pRoom.Y >= this.dragstart.Y) && (pRoom.Y <= this.dragcurrent.Y);
            }
            else if ( (dragcurrent.X <= dragstart.X) && (dragcurrent.Y <= dragstart.Y) )
            {
                bInX = (pRoom.X >= this.dragcurrent.X) && (pRoom.X <= this.dragstart.X);
                bInY = (pRoom.Y >= this.dragcurrent.Y) && (pRoom.Y <= this.dragstart.Y);
            }
            else if ((dragcurrent.X >= dragstart.X) && (dragcurrent.Y <= dragstart.Y))
            {
                bInX = (pRoom.X >= this.dragstart.X) && (pRoom.X <= this.dragcurrent.X);
                bInY = (pRoom.Y >= this.dragcurrent.Y) && (pRoom.Y <= this.dragstart.Y);
            }
            else if ((dragcurrent.X <= dragstart.X) && (dragcurrent.Y >= dragstart.Y))
            {
                bInX = (pRoom.X >= this.dragcurrent.X) && (pRoom.X <= this.dragstart.X);
                bInY = (pRoom.Y >= this.dragstart.Y) && (pRoom.Y <= this.dragcurrent.Y);
            }

            return (bInX && bInY);
        }


        private void InitWorkspaceGrid()
        {
            canvas1.Children.Clear();

            this.iLinesDrawn = 0;

            maxh = (int)Math.Floor(this.Height / 11);
            maxw = (int)Math.Floor(this.Width / 11);

            // Draw the lines for the y-axis
            for (int y = 0; y < maxh; y++)
            {
                Line myLine = new Line();
                myLine.Stroke = System.Windows.Media.Brushes.Black;
                myLine.X1 = 0;
                myLine.X2 = this.Width;
                myLine.Y1 = y * 11;
                myLine.Y2 = myLine.Y1;
                myLine.HorizontalAlignment = HorizontalAlignment.Left;
                myLine.VerticalAlignment = VerticalAlignment.Center;
                myLine.StrokeThickness = 1;
                canvas1.Children.Add(myLine);

                this.iLinesDrawn++;
            }

            // Draw the lines for the x-axis
            for (int x = 0; x < maxw; x++)
            {
                Line myLine = new Line();
                myLine.Stroke = System.Windows.Media.Brushes.Black;
                myLine.X1 = x * 11;
                myLine.X2 = myLine.X1;
                myLine.Y1 = 0;
                myLine.Y2 = this.Height;
                myLine.HorizontalAlignment = HorizontalAlignment.Left;
                myLine.VerticalAlignment = VerticalAlignment.Center;
                myLine.StrokeThickness = 1;
                canvas1.Children.Add(myLine);

                this.iLinesDrawn++;
            }
        }

        // Main drawing method
        private void DrawWorkspaceGrid()
        {
            for (int i = iLinesDrawn; i < canvas1.Children.Count; i++ )
            {
                canvas1.Children.RemoveAt(i);
            }


            // Paint the rectangles red if they are active in the dbase and green if it is the current selection
            foreach (saikomudDataSet.gridRow pRow in table.Rows)
            {
                Point p = new Point(pRow.x, pRow.y);

                bool bInSelected = (bDrag && this.isInDragSelection(p));

                if (pRow.active == 1)
                {
                    Rect r = GetRectForRoom(p);
                    Rectangle rect = new Rectangle();

                    if (bInSelected)
                    {
                        rect.Fill = System.Windows.Media.Brushes.Orange;
                    }
                    else
                    {
                        rect.Fill = System.Windows.Media.Brushes.Red;
                    }

                    if (this.selectedroom != null)
                    {
                        if ((pRow.x == this.selectedroom.X) && (pRow.y == this.selectedroom.Y))
                        {
                            rect.Fill = System.Windows.Media.Brushes.Green;
                        }
                    }
                    rect.Height = 10;
                    rect.Width = 10;
                    rect.SetValue(Canvas.LeftProperty, r.Left);
                    rect.SetValue(Canvas.TopProperty, r.Top);
                    canvas1.Children.Add(rect);
                }
                else if (bInSelected)
                {
                    Rect r = GetRectForRoom(p);
                    Rectangle rect = new Rectangle();
                    rect.Fill = System.Windows.Media.Brushes.LightGray;

                    rect.Height = 10;
                    rect.Width = 10;
                    rect.SetValue(Canvas.LeftProperty, r.Left);
                    rect.SetValue(Canvas.TopProperty, r.Top);
                    canvas1.Children.Add(rect);
                }
            }
        
        }// End of drawing

        // If a room in the dbase is active, fill in a description (currently an empty string)
        private string getRoomDescription(Point room) {
            foreach (saikomudDataSet.gridRow pRow in table.Rows) {
                if ((pRow.x == room.X) && (pRow.y == room.Y)) {
                    if ((pRow.active == 1) && !pRow.IsdescNull()) {
                        return pRow.desc;
                    } else {
                        return "";
                    }
                }
            }

            return "";
        }

        // Determine pRow
        private saikomudDataSet.gridRow getRoomRow(Point room) {
            foreach (saikomudDataSet.gridRow pRow in table.Rows) {
                if ((pRow.x == room.X) && (pRow.y == room.Y)) {
                    return pRow;
                }
            }
            return null;
        }

        // Basic startup and closing
        private void Window_Loaded(object sender, RoutedEventArgs e) {

            grideditor.saikomudDataSet saikomudDataSet = ((grideditor.saikomudDataSet)(this.FindResource("saikomudDataSet")));

        }

        private void Window_Closed(object sender, EventArgs e) {
            
            infowin.Close();
        }

        // As soon as the mainwindow is actived, redraw
        private void Window_Activated(object sender, EventArgs e) {
            InitWorkspaceGrid();

            DrawWorkspaceGrid();
        }
        
        // If a description is entered, redraw 
        public void SetRoomInfo(Point roomxy, string description) {
            adapter.Update(description, 1, (int)roomxy.X, (int)roomxy.Y, 1);

            adapter.Fill(table);

            DrawWorkspaceGrid();
        }

        /*
         *      Mouse and keyboard functions below
         */


        // When the lmb is pressed, get the position of the mouse (the room) and redraw
        private void Window_MouseLeftButtonDown(object sender, MouseButtonEventArgs e) {

            Point mouse = e.GetPosition(canvas1);
            this.selectedroom = this.GetRoomPositionByMousePos(mouse);
            this.dragstart = this.selectedroom;
            this.dragcurrent = this.selectedroom;

            infowin.setRoomInfo(this.selectedroom, getRoomDescription(this.selectedroom), this.SetRoomInfo);

            bDrag = true;

            DrawWorkspaceGrid();
        }

        // When the rmb is pressed, get the mouse position and either make the room active or inactive (and redraw)
        private void Window_MouseRightButtonDown(object sender, MouseButtonEventArgs e) {

            Point mouse = e.GetPosition(canvas1);
            this.selectedroom = this.GetRoomPositionByMousePos(mouse);

            bool bEnabled = true;

            saikomudDataSet.gridRow existingrow = getRoomRow( this.selectedroom );
            if (existingrow != null) {
                if (existingrow.active == 1) {
                    existingrow.active = 0;
                    bEnabled = false;
                } else {
                    existingrow.active = 1;
                }
                adapter.Update(existingrow);
            } else {
                adapter.Insert((int)this.selectedroom.X, (int)this.selectedroom.Y, "", 1);
            }

            adapter.Fill(table);

            DrawWorkspaceGrid();

            // Do an insert query here if this room doesn't exist
            // If it does exist, disable it
            if (bEnabled) {
                infowin.setRoomInfo(this.selectedroom, getRoomDescription(this.selectedroom), this.SetRoomInfo);
            }
        }// End of rmb-down

        private void Window_MouseLeftButtonUp(object sender, MouseButtonEventArgs e){
            bDrag = false;

            DrawWorkspaceGrid();
        }

        private void Window_MouseMove(object sender, MouseEventArgs e)
        {
            if (bDrag)
            {
                Point mouse = e.GetPosition(canvas1);
                this.dragcurrent = this.GetRoomPositionByMousePos(mouse);

                DrawWorkspaceGrid();
            }
        }

    }
}
