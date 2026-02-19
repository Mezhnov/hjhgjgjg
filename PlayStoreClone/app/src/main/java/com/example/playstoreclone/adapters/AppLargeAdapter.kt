package com.example.playstoreclone.adapters

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.bumptech.glide.Glide
import com.bumptech.glide.load.resource.bitmap.RoundedCorners
import com.example.playstoreclone.R
import com.example.playstoreclone.databinding.ItemAppLargeBinding
import com.example.playstoreclone.models.AppModel

class AppLargeAdapter(
    private val apps: List<AppModel>,
    private val onAppClick: (AppModel) -> Unit
) : RecyclerView.Adapter<AppLargeAdapter.ViewHolder>() {

    inner class ViewHolder(private val binding: ItemAppLargeBinding) :
        RecyclerView.ViewHolder(binding.root) {

        fun bind(app: AppModel) {
            binding.apply {
                tvAppName.text = app.name
                tvCategory.text = "${app.category} • ${app.rating} ★"

                // Load featured image
                Glide.with(itemView.context)
                    .load(app.featuredImageUrl.ifEmpty { app.iconUrl })
                    .placeholder(R.drawable.bg_banner)
                    .into(ivFeaturedImage)

                // Load app icon
                Glide.with(itemView.context)
                    .load(app.iconUrl)
                    .placeholder(R.drawable.ic_placeholder)
                    .transform(RoundedCorners(20))
                    .into(ivAppIcon)

                root.setOnClickListener { onAppClick(app) }
            }
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val binding = ItemAppLargeBinding.inflate(
            LayoutInflater.from(parent.context), parent, false
        )
        return ViewHolder(binding)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.bind(apps[position])
    }

    override fun getItemCount() = apps.size
}
