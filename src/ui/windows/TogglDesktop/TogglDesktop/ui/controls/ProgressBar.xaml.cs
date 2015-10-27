
namespace TogglDesktop
{
    public partial class ProgressBar
    {
        public ProgressBar()
        {
            this.DataContext = this;
            this.InitializeComponent();
        }

        public double Value
        {
            get { return this.fillRectangle.Width; }
            set { this.fillRectangle.Width = value; }
        }
        public double MaxValue
        {
            get { return this.border.Width; }
            set { this.border.Width = value; }
        }
    }
}
