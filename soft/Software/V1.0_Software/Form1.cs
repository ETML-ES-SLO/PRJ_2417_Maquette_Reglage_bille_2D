// ============================================================================
//  Références utilisées (WinForms + IO série + dessin 2D)                     //
// ============================================================================
using System;
using System.Drawing;               // Types graphiques de base
using System.Drawing.Drawing2D;     // Pour SmoothingMode.AntiAlias
using System.IO.Ports;              // Accès au port COM
using System.Text;                  // StringBuilder
using System.Windows.Forms;         // Contrôles WinForms

namespace _2417_MaquetteBille2D_Software
{
    /// <summary>Fenêtre principale (designer partiel + code ici).</summary>
    public partial class Form1 : Form
    {
        // --------------------------------------------------------------------
        // Champs « état » de l’application
        // --------------------------------------------------------------------
        private readonly SerialPort _serialPort = new SerialPort();     // Objet COM
        private readonly StringBuilder _rxBuffer = new StringBuilder(); // Tampon pour trames incomplètes

        private ushort _posX, _posY;   // Dernière position de la bille reçue
        private float _rangeX = 1023, // Pleine-échelle horizontale (valeur max X connue)
                       _rangeY = 1023; // Pleine-échelle verticale  (valeur max Y connue)

        // --------------------------------------------------------------------
        // Constructeur : initialise l’IHM et le port série
        // --------------------------------------------------------------------
        public Form1()
        {
            InitializeComponent();  // Instancie tous les contrôles du designer
            ListSerialPorts();      // Remplit la combo des ports COM disponibles
            InitSerialPort();       // Paramètre le SerialPort (vitesse, etc.)

            // Prépare la zone de dessin : ajout d’un handler pour l’événement Paint
            picBille.Paint += PicBille_Paint;
            picBille.BackColor = Color.White;  // Fond blanc pour bien voir la bille
        }

        // ====================================================================
        #region  Gestion du port série
        // ====================================================================

        /// <summary>Applique tous les paramètres au SerialPort.</summary>
        private void InitSerialPort()
        {
            _serialPort.BaudRate = 115_200;      // 115 200 baud
            _serialPort.Parity = Parity.None;  // Pas de bit de parité
            _serialPort.DataBits = 8;            // 8 bits de données
            _serialPort.StopBits = StopBits.One; // 1 bit de stop
            _serialPort.Handshake = Handshake.None; // Pas de contrôle matériel

            _serialPort.NewLine = "#";   // Délimiteur de fin de trame
            _serialPort.ReadTimeout = 200;   // ms – lève exception si dépasse
            _serialPort.WriteTimeout = 200;   // ms – idem en écriture

            // Quand des données arrivent, SerialPort déclenche DataReceived (thread I/O)
            _serialPort.DataReceived += SerialPort_DataReceived;
        }

        /// <summary>Récupère la liste des ports COM du PC et l’affiche.</summary>
        private void ListSerialPorts()
        {
            cboPorts.Items.Clear();                  // Nettoie la combo
            string[] ports = SerialPort.GetPortNames(); // e.g. « COM3 »
            Array.Sort(ports);                       // Trie alphabétiquement
            cboPorts.Items.AddRange(ports);          // Ajoute à la liste déroulante
            if (ports.Length > 0) cboPorts.SelectedIndex = 0; // Sélectionne 1er si dispo
        }

        /// <summary>Clique sur [Ouvrir/Fermer] : oscille entre ouvert et fermé.</summary>
        private void btnOpenClose_Click(object sender, EventArgs e)
        {
            if (_serialPort.IsOpen)          // -- Cas : déjà ouvert → on ferme
            {
                _serialPort.Close();
                btnOpenClose.Text = "Ouvrir";
                lblPortState.Text = "Port fermé";
            }
            else                             // -- Cas : fermé → tentative ouverture
            {
                if (cboPorts.SelectedItem == null)   // Aucun port choisi
                { MessageBox.Show("Aucun port sélectionné."); return; }

                _serialPort.PortName = cboPorts.SelectedItem.ToString(); // Ex. « COM3 »
                try
                {
                    _serialPort.Open();            // Tentative d’ouverture
                    btnOpenClose.Text = "Fermer";  // MAJ de l’IHM
                    lblPortState.Text = "Port ouvert";
                }
                catch (Exception ex)               // Erreur : mauvais port, accès refusé, …
                { MessageBox.Show("Erreur d’ouverture : " + ex.Message); }
            }
        }

        /// <summary>Fermeture de la fenêtre : s’assure que le port est fermé proprement.</summary>
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (_serialPort.IsOpen) _serialPort.Close();
        }
        #endregion

        // ====================================================================
        #region  Envoi des gains PID
        // ====================================================================

        /// <summary>Clique sur [Envoyer] : compose la trame et l’envoie.</summary>
        private void btnSendPid_Click(object sender, EventArgs e)
        {
            if (!CheckPortOpen()) return;           // Vérifie que le COM est ouvert

            // Compose « !kp=XXXXki=XXXXkd=XXXX# » ; XXXX = millièmes, 4 chiffres
            string frame =
                $"!kp={ToUInt(nudKp.Value):0000}" +
                $"ki={ToUInt(nudKi.Value):0000}" +
                $"kd={ToUInt(nudKd.Value):0000}#";

            try
            {
                _serialPort.Write(frame);           // Transmission brute
                Log("TX", frame);                   // Trace dans la liste
            }
            catch (Exception ex)                    // Timeout / port fermé…
            { MessageBox.Show("Erreur d’envoi : " + ex.Message); }
        }

        /// <summary>Convertit une valeur décimale (0–9,999) en entier sur 4 chiffres (millièmes).</summary>
        private static ushort ToUInt(decimal value) => (ushort)Math.Round(value * 1000m);

        #endregion

        // ====================================================================
        #region  Réception et décodage de trames
        // ====================================================================

        /// <summary>Callback automatique chaque fois qu’un octet arrive (thread I/O).</summary>
        private void SerialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                _rxBuffer.Append(_serialPort.ReadExisting()); // Copie tout ce qui est arrivé

                // Boucle : extraire toutes les trames complètes présentes dans le tampon
                while (true)
                {
                    string buf = _rxBuffer.ToString();
                    int posHash = buf.IndexOf('#');   // Cherche la fin de trame
                    if (posHash < 0) break;          // Pas (encore) de trame complète

                    string rawFrame = buf.Substring(0, posHash + 1); // Inclut le '#'
                    _rxBuffer.Remove(0, posHash + 1);                // Retire de l’attente

                    // Traite la trame depuis le thread UI (BeginInvoke posté dans la queue)
                    BeginInvoke(new Action(() => HandleFrame(rawFrame)));
                }
            }
            catch { /* Timeout ou Overflow : ignoré */ }
        }

        /// <summary>Analyse la trame reçue (PID ou position) et met à jour l’IHM.</summary>
        private void HandleFrame(string raw)
        {
            Log("RX", raw);                 // Ajout au journal

            string frame = raw.TrimEnd('#'); // Supprime le délimiteur

            // 1) Essaie d’abord de détecter des gains PID
            if (TryParsePid(frame, out decimal kp, out decimal ki, out decimal kd))
                ShowPid(kp, ki, kd);

            // 2) Essaie de détecter une position ou un plateau
            if (TryParsePos(frame, out ushort x, out ushort y,
                            out ushort w, out ushort h))
                ShowPos(x, y, w, h);
        }
        #endregion

        // ====================================================================
        #region  Parsing simple (pas de Regex pour gagner en perf)
        // ====================================================================

        /// <summary>Recherche "X=", "Y=" (& éventuellement "W=", "H=") dans la chaîne.</summary>
        private static bool TryParsePos(string s,
                                        out ushort x, out ushort y,
                                        out ushort w, out ushort h)
        {
            x = y = w = h = 0;                  // Valeurs par défaut

            // Localise les marqueurs
            int ix = s.IndexOf("X=");
            int iy = s.IndexOf("Y=");
            if (ix < 0 || iy < 0) return false; // Champs obligatoires absents

            // Extrait uniquement les chiffres
            string numX = ReadDigits(s, ix + 2);
            string numY = ReadDigits(s, iy + 2);

            // Convertit en ushort ; si échec, trame invalide
            if (!ushort.TryParse(numX, out x) ||
                !ushort.TryParse(numY, out y))
                return false;

            // W=... H=... : dimensions (optionnel)
            int iw = s.IndexOf("W=");
            int ih = s.IndexOf("H=");
            if (iw >= 0 && ih >= 0)
            {
                string numW = ReadDigits(s, iw + 2);
                string numH = ReadDigits(s, ih + 2);
                if (!ushort.TryParse(numW, out w) ||
                    !ushort.TryParse(numH, out h))
                    return false;
            }
            return true; // OK
        }

        /// <summary>Lit les caractères numériques consécutifs à partir d’un index.</summary>
        private static string ReadDigits(string s, int start)
        {
            var sb = new StringBuilder();
            for (int i = start; i < s.Length && char.IsDigit(s[i]); i++)
                sb.Append(s[i]);
            return sb.ToString();
        }

        /// <summary>Extrait les champs kp=XXXX / ki=XXXX / kd=XXXX (XXXX décimal).</summary>
        private static bool TryParsePid(string s,
                                        out decimal kp, out decimal ki, out decimal kd)
        {
            kp = ki = kd = 0m;

            // Repère chaque sous-chaîne
            int iKp = s.IndexOf("kp=");
            int iKi = s.IndexOf("ki=");
            int iKd = s.IndexOf("kd=");
            if (iKp < 0 || iKi < 0 || iKd < 0) return false;

            // Lit 4 chiffres après chaque préfixe
            bool ok1 = ushort.TryParse(s.Substring(iKp + 3, 4), out ushort kp_i);
            bool ok2 = ushort.TryParse(s.Substring(iKi + 3, 4), out ushort ki_i);
            bool ok3 = ushort.TryParse(s.Substring(iKd + 3, 4), out ushort kd_i);

            // Convertit en millièmes → décimal (format utilisé dans l’IHM)
            kp = kp_i / 1000m;
            ki = ki_i / 1000m;
            kd = kd_i / 1000m;

            return ok1 && ok2 && ok3;
        }
        #endregion

        // ====================================================================
        #region  Helpers d’IHM : mise à jour des contrôles
        // ====================================================================

        /// <summary>Affiche les gains PID reçus et aligne les TrackBars.</summary>
        private void ShowPid(decimal kp, decimal ki, decimal kd)
        {
            // Mise à jour NumericUpDown (précision 0.001)
            nudKp.Value = kp; nudKi.Value = ki; nudKd.Value = kd;

            // Label info
            lblPidRecv.Text = $"PID reçu : {kp:0.000} / {ki:0.000} / {kd:0.000}";

            SyncNumToTrack(); // Copie numériques → sliders
        }

        /// <summary>Met à jour la position et déclenche le repaint.</summary>
        private void ShowPos(ushort x, ushort y, ushort w, ushort h)
        {
            // Si W, H reçus (non zéro) → ces info deviennent la pleine-échelle
            if (w != 0 && h != 0) { _rangeX = w - 1; _rangeY = h - 1; }

            // Garantit que la pleine-échelle reste ≥ valeurs reçues
            if (x > _rangeX) _rangeX = x;
            if (y > _rangeY) _rangeY = y;

            _posX = x; _posY = y;                    // Stocke pour le Paint

            lblPosXY.Text = $"Position X={x}  Y={y}"; // Mise à jour label
            picBille.Invalidate();                    // Demande un redraw
        }

        /// <summary>Ajoute une ligne (horodatée) dans la ListView de log.</summary>
        private void Log(string dir, string txt)
        {
            lvwLog.Items.Add(new ListViewItem(
                new[] { $"{DateTime.Now:HH:mm:ss}", dir, txt }));
            lvwLog.EnsureVisible(lvwLog.Items.Count - 1); // Scroll jusqu’en bas
        }

        /// <summary>Affiche un message si le port n’est pas ouvert.</summary>
        private bool CheckPortOpen()
        {
            if (_serialPort.IsOpen) return true;
            MessageBox.Show("Port non ouvert."); return false;
        }
        #endregion

        // ====================================================================
        #region  Dessin de la bille rouge dans picBille
        // ====================================================================

        /// <summary>Handler Paint : dessine la bille et le cadre.</summary>
        private void PicBille_Paint(object sender, PaintEventArgs e)
        {
            Graphics g = e.Graphics;
            g.SmoothingMode = SmoothingMode.AntiAlias; // Anti-alias pour la bille

            int w = picBille.ClientSize.Width;         // Largeur en pixels
            int h = picBille.ClientSize.Height;        // Hauteur en pixels

            // Remapping : la maquette envoie X=tilt vertical, Y=tilt horizontal
            float px = _posY * w / _rangeY;            // Abscisse dans l’image
            float py = _posX * h / _rangeX;            // Ordonnée dans l’image
            const int R = 10;                          // Rayon de la bille

            // Remplissage rouge
            using (var brush = new SolidBrush(Color.Red))
                g.FillEllipse(brush, px - R, py - R, R * 2, R * 2);

            // Cadre noir autour du terrain
            using (var pen = new Pen(Color.Black))
                g.DrawRectangle(pen, 0, 0, w - 1, h - 1);
        }
        #endregion

        // ====================================================================
        #region  Synchronisation TrackBars <-> NumericUpDown pour chaque gain
        // ====================================================================

        private void Num_ValueChanged(object sender, EventArgs e) => SyncNumToTrack();
        private void Tbr_Scroll(object sender, EventArgs e) => SyncTrackToNum();

        /// <summary>Copie NumericUpDown → TrackBars (échelle *1000).</summary>
        private void SyncNumToTrack()
        {
            tbrKp.Value = (int)(nudKp.Value * 1000m);
            tbrKi.Value = (int)(nudKi.Value * 1000m);
            tbrKd.Value = (int)(nudKd.Value * 1000m);
        }

        /// <summary>Copie TrackBars → NumericUpDown.</summary>
        private void SyncTrackToNum()
        {
            nudKp.Value = tbrKp.Value / 1000m;
            nudKi.Value = tbrKi.Value / 1000m;
            nudKd.Value = tbrKd.Value / 1000m;
        }
        #endregion
    }
}
