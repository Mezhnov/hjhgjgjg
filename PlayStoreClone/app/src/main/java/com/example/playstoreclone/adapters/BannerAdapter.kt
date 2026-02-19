package com.example.playstoreclone.adapters

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.bumptech.glide.Glide
import com.bumptech.glide.load.resource.bitmap.RoundedCorners
import com.example.playstoreclone.R
import com.example.playstoreclone.databinding.ItemBannerBinding
import com.example.playstoreclone.models.BannerModel

class BannerAdapter(
    private val banners: List<BannerModel>,
    private val onBannerClick: (BannerModel) -> Unit
) : RecyclerView.Adapter<BannerAdapter.ViewHolder>() {

    inner class ViewHolder(private val binding: ItemBannerBinding) :
        RecyclerView.ViewHolder(binding.root) {

        fun bind(banner: BannerModel) {
            binding.apply {
                tvBannerTitle.text = banner.title
                tvBannerSubtitle.text = banner.subtitle

                // Load banner image
                Glide.with(itemView.context)
                    .load(banner.imageUrl)
                    .placeholder(R.drawable.bg_banner)
                    .centerCrop()
                    .into(ivBannerImage)

                // Load app icon
                Glide.with(itemView.context)
                    .load(banner.appIconUrl)
                    .placeholder(R.drawable.ic_placeholder)
                    .transform(RoundedCorners(20))
                    .into(ivAppIcon)

                root.setOnClickListener { onBannerClick(banner) }
            }
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val binding = ItemBannerBinding.inflate(
            LayoutInflater.from(parent.context), parent, false
        )
        return ViewHolder(binding)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.bind(banners[position])
    }

    override fun getItemCount() = banners.size
}
