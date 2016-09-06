package com.first.ihack102;

import android.app.Service;
import android.content.ClipData;
import android.content.ClipData.Item;
import android.content.ClipboardManager;
import android.content.ClipboardManager.OnPrimaryClipChangedListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.os.IBinder;
import android.support.v7.preference.PreferenceManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import com.geniusforapp.swag.SwagHelper.VerificationHelper;
import com.github.dubu.runtimepermissionshelper.rxver.RxPermissions;
import com.google.i18n.phonenumbers.PhoneNumberUtil;
import com.google.i18n.phonenumbers.PhoneNumberUtil.PhoneNumberFormat;
import com.google.i18n.phonenumbers.Phonenumber.PhoneNumber;
import com.okdme.menoma3ay.helpers.Countries;
import com.okdme.menoma3ay.helpers.Countries.Country;
import com.okdme.menoma3ay.model.Preferences.CountryCodePreference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

public class ClipboardMonitorService extends Service
{
  private static final String TAG = "ClipboardManager";
  String countryCode = "";
  String dailingCode = "";
  private ClipboardManager mClipboardManager;
  private ClipboardManager.OnPrimaryClipChangedListener mOnPrimaryClipChangedListener = new ClipboardManager.OnPrimaryClipChangedListener()
  {
    public void onPrimaryClipChanged()
    {
      if (PreferenceManager.getDefaultSharedPreferences(ClipboardMonitorService.this.getApplicationContext()).getBoolean("enable_contact_card", false))
      {
        String str = String.valueOf(ClipboardMonitorService.this.mClipboardManager.getPrimaryClip().getItemAt(0).getText().toString()).trim().replaceAll("\\s+", "");
        ClipboardMonitorService.this.searchQuery = str.replaceAll("[^0-9]", "");
        if (VerificationHelper.getInstance().isPhone(ClipboardMonitorService.this.searchQuery))
          ClipboardMonitorService.this.searchNumber();
      }
    }
  };
  String phone = "";
  String query = "";
  String searchQuery = "";

  private void searchNumber()
  {
    PhoneNumberUtil localPhoneNumberUtil = PhoneNumberUtil.getInstance();
    String str2;
    try
    {
      Phonenumber.PhoneNumber localPhoneNumber1 = localPhoneNumberUtil.parse(this.searchQuery, this.countryCode);
      if (localPhoneNumberUtil.isValidNumberForRegion(localPhoneNumber1, this.countryCode))
      {
        this.dailingCode = String.valueOf(localPhoneNumber1.getCountryCode());
        this.query = localPhoneNumberUtil.format(localPhoneNumber1, PhoneNumberUtil.PhoneNumberFormat.NATIONAL).replaceAll("[\\s\\-()]", "");
        search(this.query, this.dailingCode);
        Log.d("ClipboardManager", "searchNumber: valid for ragion " + this.dailingCode + this.query);
        return;
      }
      String str1 = localPhoneNumberUtil.format(localPhoneNumber1, PhoneNumberUtil.PhoneNumberFormat.INTERNATIONAL);
      str2 = localPhoneNumberUtil.getRegionCodeForNumber(localPhoneNumberUtil.parse(str1, ""));
      Log.d("ClipboardManager", "searchNumber: regeon of inrernational " + str2);
      localPhoneNumberUtil.parse(str1, str2.toUpperCase());
      if (localPhoneNumberUtil.isValidNumberForRegion(localPhoneNumber1, str2))
      {
        this.dailingCode = String.valueOf(localPhoneNumber1.getCountryCode());
        this.query = localPhoneNumberUtil.format(localPhoneNumber1, PhoneNumberUtil.PhoneNumberFormat.NATIONAL).replaceAll("[\\s\\-()]", "");
        search(this.query, this.dailingCode);
        Log.d("ClipboardManager", "searchNumber: valid internaional for regeion " + this.dailingCode + this.query);
        return;
      }
    }
    catch (Exception localException)
    {
      Log.d("ClipboardManager", "searchNumber: " + localException.toString());
      return;
    }
    Iterator localIterator = Countries.getMasterCountries().iterator();
    while (localIterator.hasNext())
    {
      Countries.Country localCountry = (Countries.Country)localIterator.next();
      if (this.searchQuery.startsWith(localCountry.getDailingCode()))
        str2 = localCountry.getCountryCode();
    }
    Phonenumber.PhoneNumber localPhoneNumber2 = localPhoneNumberUtil.parse(this.searchQuery, str2.toUpperCase());
    this.dailingCode = String.valueOf(localPhoneNumber2.getCountryCode());
    this.query = localPhoneNumberUtil.format(localPhoneNumber2, PhoneNumberUtil.PhoneNumberFormat.NATIONAL).replaceAll("[\\s\\-()]", "");
    ArrayList localArrayList = new ArrayList();
    localArrayList.addAll(Arrays.asList(getResources().getStringArray(2131361794)));
    if (localArrayList.contains(str2.toUpperCase()))
      search(this.query, this.dailingCode);
    Log.d("ClipboardManager", "searchNumber: number and international for region un valid " + this.dailingCode + this.query);
  }

  private void setDefaultCountryCode()
  {
    try
    {
      if (RxPermissions.getInstance(getApplicationContext()).isGranted("android.permission.READ_PHONE_STATE"))
      {
        this.countryCode = getCountryCodeFromSim();
        if (this.countryCode.isEmpty())
          break label79;
      }
      label79: for (this.countryCode = Countries.getCountryByCountryCode(this.countryCode).getCountryCode().toUpperCase(); ; this.countryCode = Countries.getCountryDailingCode(new CountryCodePreference(getApplicationContext()).getDefaultCountryCode()).getCountryCode().toUpperCase())
      {
        Log.d("ClipboardManager", "setDefaultCountryCode: " + this.countryCode);
        return;
      }
    }
    catch (Exception localException)
    {
      Log.d("ClipboardManager", "setDefaultCountryCode: " + localException.getMessage());
    }
  }

  public String getCountryCodeFromSim()
  {
    TelephonyManager localTelephonyManager = (TelephonyManager)getSystemService("phone");
    switch (localTelephonyManager.getSimState())
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    default:
      return "";
    case 5:
    }
    String str = localTelephonyManager.getSimCountryIso();
    localTelephonyManager.getSimOperator();
    localTelephonyManager.getSimOperatorName();
    localTelephonyManager.getSimSerialNumber();
    return str;
  }

  public IBinder onBind(Intent paramIntent)
  {
    return null;
  }

  public void onCreate()
  {
    super.onCreate();
    this.mClipboardManager = ((ClipboardManager)getSystemService("clipboard"));
    this.mClipboardManager.addPrimaryClipChangedListener(this.mOnPrimaryClipChangedListener);
    setDefaultCountryCode();
  }

  public void onDestroy()
  {
    super.onDestroy();
    if (this.mClipboardManager != null)
      this.mClipboardManager.removePrimaryClipChangedListener(this.mOnPrimaryClipChangedListener);
    startService(new Intent(getApplicationContext(), ClipboardMonitorService.class));
  }

  public void search(String paramString1, String paramString2)
  {
    Intent localIntent = new Intent(getApplicationContext(), ContactCardService.class);
    localIntent.putExtra("number", paramString1);
    localIntent.putExtra("countryCode", paramString2);
    startService(localIntent);
  }
}
