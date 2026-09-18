// ===================  Form1.Designer.cs  ===================
using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace _2417_MaquetteBille2D_Software
{
    partial class Form1
    {
        private IContainer components = null;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null)) components.Dispose();
            base.Dispose(disposing);
        }

        #region Windows‑Form Designer generated code
        private void InitializeComponent()
        {
            this.components = new Container();
            this.cboPorts = new ComboBox();
            this.btnOpenClose = new Button();
            this.lblPortState = new Label();

            this.nudKp = new NumericUpDown();
            this.nudKi = new NumericUpDown();
            this.nudKd = new NumericUpDown();
            this.lblPidRecv = new Label();
            this.btnSendPid = new Button();

            this.tbrKp = new TrackBar();
            this.tbrKi = new TrackBar();
            this.tbrKd = new TrackBar();
            this.lblKp = new Label();
            this.lblKi = new Label();
            this.lblKd = new Label();

            this.picBille = new PictureBox();
            this.lblPosXY = new Label();

            this.lvwLog = new ListView();
            this.lblLog = new Label();

            this.serialPort1 = new System.IO.Ports.SerialPort(this.components);

            ((ISupportInitialize)(this.nudKp)).BeginInit();
            ((ISupportInitialize)(this.nudKi)).BeginInit();
            ((ISupportInitialize)(this.nudKd)).BeginInit();
            ((ISupportInitialize)(this.tbrKp)).BeginInit();
            ((ISupportInitialize)(this.tbrKi)).BeginInit();
            ((ISupportInitialize)(this.tbrKd)).BeginInit();
            ((ISupportInitialize)(this.picBille)).BeginInit();
            this.SuspendLayout();

            // cboPorts
            this.cboPorts.DropDownStyle = ComboBoxStyle.DropDownList;
            this.cboPorts.Location = new Point(25, 25);
            this.cboPorts.Size = new Size(120, 32);

            // btnOpenClose
            this.btnOpenClose.Location = new Point(160, 25);
            this.btnOpenClose.Size = new Size(120, 32);
            this.btnOpenClose.Text = "Ouvrir";
            this.btnOpenClose.Click += new EventHandler(this.btnOpenClose_Click);

            // lblPortState
            this.lblPortState.AutoSize = true;
            this.lblPortState.Location = new Point(300, 30);
            this.lblPortState.Text = "Port fermé";

            // NumericUpDowns
            NumericUpDown[] nuds = { nudKp, nudKi, nudKd };
            Label[] lbls = { lblKp, lblKi, lblKd };
            string[] lblTxt = { "Kp", "Ki", "Kd" };
            TrackBar[] tbrs = { tbrKp, tbrKi, tbrKd };
            for (int i = 0; i < 3; i++)
            {
                nuds[i].DecimalPlaces = 3;
                nuds[i].Increment = 0.001m;
                nuds[i].Maximum = 10;
                nuds[i].Location = new Point(80, 100 + i * 70);
                nuds[i].Size = new Size(80, 30);
                nuds[i].ValueChanged += this.Num_ValueChanged;

                lbls[i].Location = new Point(25, 105 + i * 70);
                lbls[i].AutoSize = true;
                lbls[i].Text = lblTxt[i];

                tbrs[i].Location = new Point(180, 90 + i * 70);
                tbrs[i].Width = 250;
                tbrs[i].Maximum = 10000;           // 0‑10 avec 3 déc = 0‑10000
                tbrs[i].TickFrequency = 1000;
                tbrs[i].Scroll += this.Tbr_Scroll;
            }

            // lblPidRecv
            this.lblPidRecv.AutoSize = true;
            this.lblPidRecv.Location = new Point(25, 310);
            this.lblPidRecv.Text = "PID reçu : — / — / —";

            // btnSendPid
            this.btnSendPid.Location = new Point(25, 340);
            this.btnSendPid.Size = new Size(140, 35);
            this.btnSendPid.Text = "Envoyer PID";
            this.btnSendPid.Click += new EventHandler(this.btnSendPid_Click);

            // picBille
            this.picBille.Location = new Point(480, 25);
            this.picBille.Size = new Size(300, 200);
            this.picBille.BorderStyle = BorderStyle.FixedSingle;

            // lblPosXY
            this.lblPosXY.AutoSize = true;
            this.lblPosXY.Location = new Point(480, 235);
            this.lblPosXY.Text = "Position X=0000  Y=0000";

            // lvwLog
            this.lvwLog.Location = new Point(25, 400);
            this.lvwLog.Size = new Size(755, 180);
            this.lvwLog.View = View.Details;
            this.lvwLog.Columns.Add("Horloge", 90);
            this.lvwLog.Columns.Add("Direction", 60);
            this.lvwLog.Columns.Add("Trame", 580);

            // lblLog
            this.lblLog.AutoSize = true;
            this.lblLog.Location = new Point(25, 372);
            this.lblLog.Text = "Journal TX / RX";

            // Form1
            this.ClientSize = new Size(820, 600);
            this.Controls.AddRange(new Control[]
            {
                cboPorts, btnOpenClose, lblPortState,
                nudKp, nudKi, nudKd, lblKp, lblKi, lblKd,
                tbrKp, tbrKi, tbrKd, lblPidRecv, btnSendPid,
                picBille, lblPosXY, lvwLog, lblLog
            });
            this.Text = "Maquette Bille 2D";
            this.FormClosing += this.Form1_FormClosing;
            ((ISupportInitialize)(this.nudKp)).EndInit();
            ((ISupportInitialize)(this.nudKi)).EndInit();
            ((ISupportInitialize)(this.nudKd)).EndInit();
            ((ISupportInitialize)(this.tbrKp)).EndInit();
            ((ISupportInitialize)(this.tbrKi)).EndInit();
            ((ISupportInitialize)(this.tbrKd)).EndInit();
            ((ISupportInitialize)(this.picBille)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();
        }
        #endregion

        // Designer fields
        private ComboBox cboPorts;
        private Button btnOpenClose;
        private Label lblPortState;

        private NumericUpDown nudKp;
        private NumericUpDown nudKi;
        private NumericUpDown nudKd;
        private Label lblKp;
        private Label lblKi;
        private Label lblKd;
        private Label lblPidRecv;
        private Button btnSendPid;

        private TrackBar tbrKp;
        private TrackBar tbrKi;
        private TrackBar tbrKd;

        private PictureBox picBille;
        private Label lblPosXY;

        private ListView lvwLog;
        private Label lblLog;

        private System.IO.Ports.SerialPort serialPort1;
    }
}