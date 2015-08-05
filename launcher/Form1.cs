using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;

namespace launcher
{
    public partial class Form1 : Form
    {
        private static string[] MiningURL = { "axiom.eu.nicehash.com", "axiom.usa.nicehash.com", 
                                                "axiom.hk.nicehash.com", "axiom.jp.nicehash.com" };

        private static string[] Executables = { "cpuminer_x64_SSE2_4way.exe", "cpuminer_x64_AVX_4way.exe", "cpuminer_x64_AVX2_8way.exe" };

        private static Process CPUMiner;
        private static Timer T;
        private static Boolean FirstTime;


        public Form1()
        {
            InitializeComponent();

            Text += " v" + Application.ProductVersion;
            
            T = new Timer();
            T.Tick += T_Tick;
            T.Interval = 60000;

            FirstTime = false;

            comboBox1.SelectedIndex = Config.ConfigData.Location;
            textBox1.Text = Config.ConfigData.BitcoinAddress;
            textBox2.Text = Config.ConfigData.WorkerName;
            comboBox2.SelectedIndex = Config.ConfigData.Extension;
            numericUpDown1.Value = Config.ConfigData.Threads;
        }


        private void T_Tick(object sender, EventArgs e)
        {
            if (!VerifyMiningAddress()) return;

            double Rate = NiceHashStats.GetAxiomRate();
            NiceHashStats.nicehash_stats NHS = NiceHashStats.GetStats(textBox1.Text.Trim(), comboBox1.SelectedIndex);

            if (Rate > 0 && NHS != null)
            {
                NHS.accepted_speed *= 1000000; // from GHs to kHs
                toolStripStatusLabel2.Text = NHS.accepted_speed.ToString("F2");
                toolStripStatusLabel4.Text = (Rate * NHS.accepted_speed).ToString("F8");
                toolStripStatusLabel6.Text = NHS.balance.ToString("F8");
            }
        }


        private void linkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            if (!VerifyMiningAddress()) return;

            int location = comboBox1.SelectedIndex;
            if (location > 1) location = 1;

            System.Diagnostics.Process.Start("http://www.nicehash.com/index.jsp?p=miners&addr=" + textBox1.Text.Trim() + "&a=13&l=" + location.ToString());
        }


        private bool VerifyMiningAddress()
        {
            if (!BitcoinAddress.ValidateBitcoinAddress(textBox1.Text.Trim()))
            {
                MessageBox.Show("Invalid Bitcoin address!\n\nPlease, enter valid Bitcoin address.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Stop);
                textBox1.Focus();
                return false;
            }

            return true;
        }


        private void button1_Click(object sender, EventArgs e)
        {
            if (CPUMiner != null)
            {
                // stop miner
                CPUMiner.Kill();
                CPUMiner.Close();
                CPUMiner = null;

                ResetButtonText();
                return;
            }
            
            if (!VerifyMiningAddress()) return;

            button1.Text = "Please wait...";
            button1.Update();

            string Worker = textBox2.Text.Trim();
            if (Worker.Length > 0)
                Worker = textBox1.Text.Trim() + "." + Worker;
            else
                Worker = textBox1.Text.Trim();

            string FileName = "bin\\" + Executables[comboBox2.SelectedIndex];

            string CommandLine = "--algo=axiom --url=stratum+tcp://" + MiningURL[comboBox1.SelectedIndex] + ":3346 --userpass=" + Worker + ":d=0.0000008 --threads=" + numericUpDown1.Value.ToString();

            CPUMiner = Process.Start(FileName, CommandLine);
            CPUMiner.EnableRaisingEvents = true;
            CPUMiner.Exited += CPUMiner_Exited;

            if (!FirstTime)
            {
                T.Start();
                FirstTime = true;
                T_Tick(null, null);
            }

            Config.ConfigData.BitcoinAddress = textBox1.Text.Trim();
            Config.ConfigData.WorkerName = textBox2.Text.Trim();
            Config.ConfigData.Location = comboBox1.SelectedIndex;
            Config.ConfigData.Threads = decimal.ToInt32(numericUpDown1.Value);
            Config.ConfigData.Extension = comboBox2.SelectedIndex;
            Config.Commit();

            button1.Text = "Stop";
        }

        
        private void CPUMiner_Exited(object sender, EventArgs e)
        {
            CPUMiner.Close();
            CPUMiner = null;

            ResetButtonText();
        }


        delegate void ResetButtonTextCallback();

        private void ResetButtonText()
        {
            if (this.button1.InvokeRequired)
            {
                ResetButtonTextCallback d = new ResetButtonTextCallback(ResetButtonText);
                this.Invoke(d, new object[] { });
            }
            else
            {
                this.button1.Text = "Start";
            }
        }

        private void linkLabel2_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            System.Diagnostics.Process.Start("http://www.nicehash.com");
        }
    }
}
