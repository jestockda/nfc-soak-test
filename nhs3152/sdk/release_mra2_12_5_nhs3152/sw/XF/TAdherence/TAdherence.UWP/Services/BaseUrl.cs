/*
 * Copyright 2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

using TAdherence.UWP;

[assembly: Xamarin.Forms.Dependency(typeof(BaseUrl))]
namespace TAdherence.UWP
{
    public class BaseUrl : Interfaces.IBaseUrl
    {
        public string Get()
        {
            return "ms-appx-web:///";
        }
    }
}
