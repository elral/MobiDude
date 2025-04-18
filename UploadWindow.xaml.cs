﻿using System.Windows;

namespace MobiDude_V2
{
    public partial class UploadWindow : Window
    {
        public UploadWindow()
        {
            InitializeComponent();
        }

        public void AppendLine(string line)
        {
            Dispatcher.Invoke(() =>
            {
                OutputTextBox.AppendText(line + "\n");
                OutputTextBox.ScrollToEnd();
            });
        }
    }
}
