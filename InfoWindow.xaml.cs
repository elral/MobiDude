using System.Windows;

namespace MobiDude_V2 // Hier sollte dein Projektname oder Namespace stehen
{
    public partial class InfoWindow : Window
    {
        public InfoWindow(string info, string config)
        {
            InitializeComponent(); // Diese Methode lädt das Layout aus der XAML
            InfoTextBox.Text = info;
            ConfigTextBox.Text = config;
        }
    }
}
