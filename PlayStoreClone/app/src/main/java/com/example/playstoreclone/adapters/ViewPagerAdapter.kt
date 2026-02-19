package com.example.playstoreclone.adapters

import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentActivity
import androidx.viewpager2.adapter.FragmentStateAdapter
import com.example.playstoreclone.fragments.HomeFragment

class ViewPagerAdapter(
    fragmentActivity: FragmentActivity
) : FragmentStateAdapter(fragmentActivity) {

    private val tabTitles = listOf(
        "For you", "Top charts", "Kids", "Premium", "Categories"
    )

    override fun getItemCount(): Int = tabTitles.size

    override fun createFragment(position: Int): Fragment {
        return HomeFragment.newInstance(tabTitles[position])
    }

    fun getTabTitle(position: Int): String = tabTitles[position]
}
