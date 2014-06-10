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
        protected saikomudDataSetTableAdapters.environmentTableAdapter adapterEnv;
        protected saikomudDataSet.environmentDataTable tableEnv;
        protected Point selectedroom;

        public MainWindow() {
            InitializeComponent();

            infowin = new Info();
            infowin.Show();

            adapter = new saikomudDataSetTableAdapters.gridTableAdapter();
            table = new saikomudDataSet.gridDataTable();

            adapterEnv = new saikomudDataSetTableAdapters.environmentTableAdapter();
            tableEnv = new saikomudDataSet.environmentDataTable();

            adapter.Fill(table);
            adapterEnv.Fill(tableEnv);

            infowin.feedEnvtypes(tableEnv);
        }

        public Color getFlatColorByEnv(uint env) {
            foreach (saikomudDataSet.environmentRow pRow in tableEnv.Rows) {
                if ( pRow.id == env ) {
                    uint rgb = uint.Parse( pRow.flatcolor, System.Globalization.NumberStyles.AllowHexSpecifier);
                    return Color.FromRgb( (byte)((rgb >> 16) & 255), (byte)((rgb >> 8) & 255), (byte)(rgb & 255));
                }
            }

            return Color.FromRgb(255,255,255);
        }

        private Point GetRoomPositionByMousePos( Point mouse ) {
            Point p = new Point();

            p.X = Math.Floor(mouse.X / 11.0);
            p.Y = Math.Floor(mouse.Y / 11.0);

            p.X -= Math.Floor(maxw / 2.0);
            p.Y = Math.Floor(maxh / 2.0) - p.Y;

            return p;
        }

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

        private void DrawWorkspaceGrid() {
            canvas1.Children.Clear();

            maxh = (int)Math.Floor( this.Height / 11 );
            maxw = (int)Math.Floor( this.Width / 11 );
            for (int y = 0; y < maxh; y++) {
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
            }

            for (int x = 0; x < maxw; x++) {
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
            }
            
            foreach (saikomudDataSet.gridRow pRow in table.Rows) {
                Rect r = GetRectForRoom(new Point(pRow.x, pRow.y));
                Rectangle rect = new Rectangle();
                rect.Fill = new SolidColorBrush(getFlatColorByEnv(pRow.envtype));
                if (this.selectedroom != null) {
                    if ((pRow.x == this.selectedroom.X) && (pRow.y == this.selectedroom.Y)) {
                        rect.Fill = System.Windows.Media.Brushes.Green;
                    }
                }
                rect.Height = 10;
                rect.Width = 10;
                rect.SetValue(Canvas.LeftProperty, r.Left);
                rect.SetValue(Canvas.TopProperty, r.Top);
                canvas1.Children.Add(rect);

                if (pRow.active != 1) {
                    Line myLine1 = new Line();
                    Line myLine2 = new Line();
                    myLine1.X1 = r.Left;
                    myLine1.Y1 = r.Top;
                    myLine1.X2 = r.Left + 10;
                    myLine1.Y2 = r.Top + 10;
                    myLine2.X1 = r.Left + 10;
                    myLine2.Y1 = r.Top;
                    myLine2.X2 = r.Left;
                    myLine2.Y2 = r.Top + 10;
                    myLine1.StrokeThickness = 1;
                    myLine2.StrokeThickness = 1;
                    myLine1.Stroke = System.Windows.Media.Brushes.Black;
                    myLine2.Stroke = System.Windows.Media.Brushes.Black;
                    canvas1.Children.Add(myLine1);
                    canvas1.Children.Add(myLine2);
                }
            }
        }

        private saikomudDataSet.gridRow getRoomRow(Point room) {
            foreach (saikomudDataSet.gridRow pRow in table.Rows) {
                if ((pRow.x == room.X) && (pRow.y == room.Y)) {
                    return pRow;
                }
            }

            return null;
        }

        private void Window_Activated(object sender, EventArgs e) {
            DrawWorkspaceGrid();
        }

        public void SetRoomInfo(Point roomxy, string description, uint envtype, bool traversable) {
            byte itrav = 0;
            if (traversable) {
                itrav = 1;
            }
            adapter.UpdateQuery2((int)roomxy.X, (int)roomxy.Y, description, itrav, (int)envtype, (int)roomxy.X, (int)roomxy.Y);

            adapter.Fill(table);

            DrawWorkspaceGrid();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e) {
            infowin.Left = this.Left - infowin.Width - 10;
            infowin.Top = this.Top;

            grideditor.saikomudDataSet saikomudDataSet = ((grideditor.saikomudDataSet)(this.FindResource("saikomudDataSet")));

        }

        private void Window_Closed(object sender, EventArgs e) {
            infowin.Close();
        }

        protected void LoadSelectedRoom(bool bToggleActive) {
            uint envtype = 1;
            string desc = "";
            bool traversable = true;

            saikomudDataSet.gridRow existingrow = getRoomRow(this.selectedroom);
            if (existingrow == null) {
                adapter.Insert((int)this.selectedroom.X, (int)this.selectedroom.Y, "", 1, 1);
            } else {
                desc = existingrow.desc;
                envtype = existingrow.envtype;
                traversable = (existingrow.active == 1);
            }

            if (bToggleActive) {
                traversable = !traversable;
            }

            adapter.Fill(table);

            DrawWorkspaceGrid();

            infowin.setRoomInfo(this.selectedroom, desc, envtype, traversable, this.SetRoomInfo);
            if (bToggleActive) {
                infowin.save();
            }
        }

        private void Window_MouseLeftButtonDown(object sender, MouseButtonEventArgs e) {
            Point mouse = e.GetPosition(canvas1);
            this.selectedroom = this.GetRoomPositionByMousePos(mouse);

            LoadSelectedRoom(false);
        }

        private void Window_MouseRightButtonDown(object sender, MouseButtonEventArgs e) {
            Point mouse = e.GetPosition(canvas1);
            this.selectedroom = this.GetRoomPositionByMousePos(mouse);
            
            LoadSelectedRoom(true);
        }

        private void Window_KeyDown(object sender, KeyEventArgs e) {
            if (e.Key == Key.Up) {
                this.selectedroom.Y += 1;
            } else if (e.Key == Key.Down) {
                this.selectedroom.Y -= 1;
            } else if (e.Key == Key.Left) {
                this.selectedroom.X -= 1;
            } else if (e.Key == Key.Right) {
                this.selectedroom.X += 1;
            }

            LoadSelectedRoom(false);
        }

        private void Window_MouseMove(object sender, MouseEventArgs e)
        {

        }

        private void Window_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {

        }
    }
}
