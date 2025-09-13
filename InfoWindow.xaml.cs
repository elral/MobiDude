using System.Windows;

namespace MobiDude_V2
{
    public partial class InfoWindow : Window
    {
        public InfoWindow(string info, string config)
        {
            InitializeComponent();
            InfoTextBox.Text = info;
            ConfigTextBox.Text = config;
        }
    }
}
