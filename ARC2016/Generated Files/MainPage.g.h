﻿#pragma once
//------------------------------------------------------------------------------
//     This code was generated by a tool.
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
//------------------------------------------------------------------------------


namespace Windows {
    namespace UI {
        namespace Xaml {
            namespace Controls {
                ref class CaptureElement;
                ref class Image;
                ref class CheckBox;
                ref class Button;
                ref class TextBlock;
                ref class TextBox;
            }
        }
    }
}

namespace ARC2016
{
    [::Windows::Foundation::Metadata::WebHostHidden]
    partial ref class MainPage : public ::Windows::UI::Xaml::Controls::Page, 
        public ::Windows::UI::Xaml::Markup::IComponentConnector,
        public ::Windows::UI::Xaml::Markup::IComponentConnector2
    {
    public:
        void InitializeComponent();
        virtual void Connect(int connectionId, ::Platform::Object^ target);
        virtual ::Windows::UI::Xaml::Markup::IComponentConnector^ GetBindingConnector(int connectionId, ::Platform::Object^ target);
    
    private:
        bool _contentLoaded;
    
        private: ::Windows::UI::Xaml::Controls::CaptureElement^ ImgCamera;
        private: ::Windows::UI::Xaml::Controls::Image^ ImgCapture;
        private: ::Windows::UI::Xaml::Controls::CheckBox^ ChkDisplay;
        private: ::Windows::UI::Xaml::Controls::Button^ btnShutdown;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ Distance1;
        private: ::Windows::UI::Xaml::Controls::TextBox^ txtDistance1;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ Distance2;
        private: ::Windows::UI::Xaml::Controls::TextBox^ txtDistance2;
        private: ::Windows::UI::Xaml::Controls::Button^ btnSensor;
        private: ::Windows::UI::Xaml::Controls::CheckBox^ chkSensorSimulation;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ AccelX;
        private: ::Windows::UI::Xaml::Controls::TextBox^ txtAccelX;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ AccelY;
        private: ::Windows::UI::Xaml::Controls::TextBox^ txtAccelY;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ AccelZ;
        private: ::Windows::UI::Xaml::Controls::TextBox^ txtAccelZ;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ AngleX;
        private: ::Windows::UI::Xaml::Controls::TextBox^ txtAngleX;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ AngleY;
        private: ::Windows::UI::Xaml::Controls::TextBox^ txtAngleY;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ Threshold;
        private: ::Windows::UI::Xaml::Controls::TextBox^ txtBinaryThreshold;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ CameraAlive;
        private: ::Windows::UI::Xaml::Controls::TextBox^ txtCameraAlive;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ Target;
        private: ::Windows::UI::Xaml::Controls::TextBox^ txtTarget;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ Duration;
        private: ::Windows::UI::Xaml::Controls::TextBox^ txtCameraDuration;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ CornerIndex;
        private: ::Windows::UI::Xaml::Controls::TextBox^ txtCornerIndex;
    };
}

